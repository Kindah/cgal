// Copyright (c) 2005, 2006 Fernando Luis Cacciola Carballal. All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: $
// $Id: $
//
// Author(s)     : Fernando Cacciola <fernando_cacciola@ciudad.com.ar>
//
#ifndef CGAL_SURFACE_MESH_SIMPLIFICATION_TSMS_COMMON_H
#define CGAL_SURFACE_MESH_SIMPLIFICATION_TSMS_COMMON_H 1

#include <functional>
#include <utility>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map.hpp>
#include <boost/optional/optional.hpp>
#include <boost/none.hpp>
#include <boost/tuple/tuple.hpp>

CGAL_BEGIN_NAMESPACE

namespace Triangulated_surface_mesh { namespace Simplification 
{

template<class TSM> 
struct Surface_geometric_traits 
{ 
  typedef typename TSM::Point_3 Point_3 ;
  
  typedef typename Kernel_traits<Point_3>::Kernel Kernel ;
  
  typedef typename Kernel::FT FT ;
} ;

} } // namespace Triangulated_surface_mesh::Simplification

//
// Valid surface predicate
//
template<class TSM>
inline bool is_valid_triangulated_surface_mesh ( TSM const& aTSM ) { return aTSM.is_pure_triangle() ; }

CGAL_END_NAMESPACE

#ifdef CGAL_SURFACE_SIMPLIFICATION_ENABLE_TRACE
#define CGAL_TSMS_ENABLE_TRACE
#endif

#ifdef CGAL_TSMS_ENABLE_TRACE

#  include<string>
#  include<iostream>
#  include<sstream>
#  define CGAL_TSMS_TRACE_IMPL(m) \
     { \
       std::ostringstream ss ; ss << m ; std::string s = ss.str(); \
       Surface_simplification_external_trace(s); \
     }
#endif

#ifdef CGAL_SURFACE_SIMPLIFICATION_ENABLE_TRACE
#  define CGAL_TSMS_TRACE(l,m) if ( l <= CGAL_SURFACE_SIMPLIFICATION_ENABLE_TRACE ) CGAL_TSMS_TRACE_IMPL(m)
#else
#  define CGAL_TSMS_TRACE(l,m)
#endif

#undef CGAL_TSMS_ENABLE_TRACE

#endif // CGAL_SURFACE_MESH_SIMPLIFICATION_TSMS_COMMON_H //
// EOF //
 
