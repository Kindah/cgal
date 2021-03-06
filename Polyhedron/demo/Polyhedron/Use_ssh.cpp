// Copyright (c) 2020  GeometryFactory (France).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL$
// $Id$
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
// 
//
// Author(s)     : Maxime Gimeno

#ifdef CGAL_USE_SSH

#include <CGAL/Three/Three.h>

#include "CGAL/Use_ssh.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>
#include <chrono>
#include <thread>

#include <QMessageBox>

bool test_result(int res)
{
  switch(res){

  case SSH_AUTH_ERROR:
  {
    std::cerr<<"Auth failed with error: "<<std::endl;
    return false;
  }
  case SSH_AUTH_DENIED:
  {
    std::cerr<<"The server doesn't accept that public key as an authentication token. Try another key or another method."<<std::endl;
    return false;
  }
  case SSH_AUTH_PARTIAL :
  {
    std::cerr<<"You've been partially authenticated, you still have to use another method."<<std::endl;
    return false;
  }
  case SSH_OK:
    return true;
  case SSH_EOF:
    std::cerr<<"key doesn't exist."<<std::endl;
    return false;
  default:
    return false;
  }
  return true;
}
namespace CGAL{
namespace ssh_internal{

bool establish_ssh_session(ssh_session &session,
                           const char* user,
                           const char* server,
                           const char* pub_key_path,
                           const char* priv_key_path,
                           const char* priv_key_password)
{
  int port = 22;

  //Can use SSH_LOG_PROTOCOL here for verbose output
  int verbosity = SSH_LOG_NOLOG;
  int res;
  //retry 4 times max each time the connection asks to be retried.
  for(int k = 0; k < 4; ++k)
  {
    session = ssh_new();
    ssh_options_set( session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity );
    ssh_options_set( session, SSH_OPTIONS_PORT, &port );
    ssh_options_set( session, SSH_OPTIONS_USER, user );
    ssh_options_set( session, SSH_OPTIONS_HOST, server);
    
    ssh_connect(session);
#if LIBSSH_VERSION_MAJOR <1 && LIBSSH_VERSION_MINOR < 8
    if( ssh_is_server_known(session) != SSH_SERVER_KNOWN_OK )
#else
      if( ssh_session_is_known_server(session) != SSH_KNOWN_HOSTS_OK )
#endif
    {
      if(QMessageBox::warning(CGAL::Three::Three::mainWindow(), QString("Unknown Server"),
                              QString ("The server you are trying to join is not known.\n"
                                       "Do you wish to add it to the known servers list and continue?"),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
      {
        return false;
      }
#if LIBSSH_VERSION_MAJOR <1 && LIBSSH_VERSION_MINOR < 8
      if( ssh_write_knownhost(session) != SSH_OK )
#else
      if( ssh_session_update_known_hosts(session) != SSH_OK )
#endif
      {
        std::cerr << "writeKnownHost failed" << std::endl;
        return false;
      }
      else
      {
        ssh_connect(session);
      }
    }
    ssh_key pubkey = ssh_key_new();
    ssh_pki_import_pubkey_file(pub_key_path, &pubkey);
    res = ssh_userauth_try_publickey(session, NULL, pubkey);
    if(res == SSH_AUTH_AGAIN)
      ssh_disconnect(session);
    else
      break;
  }
  
  if(!test_result(res))
  {
    ssh_disconnect(session);
    return false;
  }

  ssh_key privkey = ssh_key_new();
  res = ssh_pki_import_privkey_file(priv_key_path, priv_key_password, NULL, NULL, &privkey);
  if (!test_result(res))
  {
    ssh_disconnect(session);
    return false;
  }
  res = ssh_userauth_publickey(session, NULL, privkey);
  if(!test_result(res))
  {
    ssh_disconnect(session);
    return false;
  }
  return true;
}

void close_connection(ssh_session &session)
{
  ssh_disconnect(session);
}

bool push_file(ssh_session &session,
               const char* dest_path,
               const char* filepath)
{
  //copy a file
  ssh_scp scp = ssh_scp_new(
        session, SSH_SCP_WRITE | SSH_SCP_RECURSIVE, "/tmp");
  if (scp == NULL)
  {
    std::cerr<<"Error allocating scp session: %s\n"
            << ssh_get_error(session)<<std::endl;
    return false;
  }
  int res = ssh_scp_init(scp);
  if(res != SSH_OK)
  {
    std::cerr<< "Error initializing scp session: %s\n"
             << ssh_get_error(session)<<std::endl;
    ssh_scp_free(scp);
    ssh_disconnect(session);
    return false;
  }
  //read a file into a buffer
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if(!file)
  {
    std::cerr<<"File not found."<<std::endl;
    ssh_scp_free(scp);
    ssh_disconnect(session);
    return false;
  }
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer(size);
  if (!file.read(buffer.data(), size))
  {
    std::cerr<<"error while reading file."<<std::endl;
    ssh_disconnect(session);
    return false;
  }
  //push a file to /tmp
  res = ssh_scp_push_directory(scp, ".", 0707);
  if (res != SSH_OK)
  {
    std::cerr<<"Can't create remote directory: %s\n"
            <<ssh_get_error(session)<<std::endl;
    ssh_disconnect(session);
    return false;
  }
  res = ssh_scp_push_file
      (scp, dest_path, size, 0707);
  if (res != SSH_OK)
  {
    std::cerr<< "Can't open remote file: %s\n"
             << ssh_get_error(session)<<std::endl;
    ssh_disconnect(session);
    return false;
  }
  res = ssh_scp_write(scp, buffer.data(), size);
  //some versions of libssh don't copy everything without this.
  //This is the case for the official version on Ubuntu 18.04
  std::chrono::duration<int, std::micro> timespan(size);
  std::this_thread::sleep_for(timespan);
  if (res != SSH_OK)
  {
    std::cerr<< "Can't write to remote file: %s\n"
             << ssh_get_error(session)<<std::endl;
    ssh_disconnect(session);
    return false;
  }
  return true;
}

bool pull_file(ssh_session &session,
               const char* from_path,
               const char* to_path)
{
  int rc;
  std::size_t size;
  std::size_t processed = 0;
  std::vector<char> buffer;

  ssh_scp scp = ssh_scp_new(
        session, SSH_SCP_READ | SSH_SCP_RECURSIVE, from_path);
  if (scp == NULL)
  {
    std::cerr<<"Error allocating scp session: %s\n"
            << ssh_get_error(session)<<std::endl;
    return false;
  }
  int res = ssh_scp_init(scp);
  if(res != SSH_OK)
  {
    std::cerr<< "Error initializing scp session: %s\n"
             << ssh_get_error(session)<<std::endl;
    ssh_scp_free(scp);
    ssh_disconnect(session);
    return false;
  }
  rc = ssh_scp_pull_request(scp);
  if (rc != SSH_SCP_REQUEST_NEWFILE)
  {
    std::cerr<< "Error receiving information about file: %s\n"
             << ssh_get_error(session)<<std::endl;
    ssh_scp_free(scp);
    ssh_disconnect(session);
    return false;
  }
  size = ssh_scp_request_get_size64(scp);
  buffer.resize(size);
  if(ssh_scp_accept_request(scp) != SSH_OK)
  {
    std::cerr<< "Could not accept request."<<std::endl;
    ssh_scp_free(scp);
    ssh_disconnect(session);
    return false;
  }

  do{
    rc = ssh_scp_read(scp, buffer.data() + processed, size-processed);
    if (rc == SSH_ERROR)
    {
      std::cerr<< "Error receiving file data: %s\n"<< ssh_get_error(session)<<std::endl;
      //free(buffer);
      ssh_scp_free(scp);
      ssh_disconnect(session);
      return false;
    }
    else
      processed += rc;
  }while(processed != size);
  std::ofstream file(to_path, std::ios::binary |std::ios::trunc);
  if(!file.write(buffer.data(), size))
  {
    std::cerr<<"Error while writing file."<<std::endl;
  }
  file.close();
  ssh_scp_free(scp);
  return true;
}

}}
#endif
