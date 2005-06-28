// Copyright (c) 2005  Tel-Aviv University (Israel).
// All rights reserved.
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
// $Source$
// $Revision$ $Date$
// $Name$
//
// Author(s)     : Ron Wein          <wein@post.tau.ac.il>

#ifndef CGAL_ARR_CONIC_TRAITS_2_H
#define CGAL_ARR_CONIC_TRAITS_2_H

/*! \file
 * The conic traits-class for the arrangement package.
 */

#include <CGAL/tags.h>
#include <CGAL/Arr_traits_2/Conic_arc_2.h>
#include <CGAL/Arr_traits_2/Conic_x_monotone_arc_2.h>
#include <CGAL/Arr_traits_2/Conic_point_2.h>

#include <fstream>

CGAL_BEGIN_NAMESPACE

/*!
 * \class A traits class for maintaining an arrangement of conic arcs (bounded
 * segments of algebraic curves of degree 2 at most).
 *
 * The class is templated with two parameters: 
 * Rat_kernel A kernel that provides the input objects or coefficients.
 *            Rat_kernel::FT should be an integral or a rational type.
 * Alg_kernel A geometric kernel, where Alg_kernel::FT is the number type
 *            for the coordinates of arrangement vertices, which are algebraic
 *            numbers of degree up to 4 (preferably it is CORE::Expr).
 * Nt_traits A traits class for performing various operations on the integer,
 *           rational and algebraic types. 
 */
template <class Rat_kernel_, class Alg_kernel_, class Nt_traits_>
class Arr_conic_traits_2 
{
public:

  typedef Rat_kernel_                     Rat_kernel;
  typedef Alg_kernel_                     Alg_kernel;
  typedef Nt_traits_                      Nt_traits;

  typedef typename Rat_kernel::FT         Rational;
  typedef typename Rat_kernel::Point_2    Rat_point_2;
  typedef typename Rat_kernel::Segment_2  Rat_segment_2;
  typedef typename Rat_kernel::Line_2     Rat_line_2;
  typedef typename Rat_kernel::Circle_2   Rat_circle_2;

  typedef typename Alg_kernel::FT         Algebraic;

  typedef typename Nt_traits::Integer     Integer;

  // Category tags:
  typedef Tag_true                        Has_left_category;
  typedef Tag_true                        Has_merge_category;

  // Traits objects:
  typedef _Conic_arc_2<Rat_kernel, Alg_kernel, Nt_traits> Curve_2;
  typedef _Conic_x_monotone_arc_2<Curve_2>                X_monotone_curve_2;
  typedef _Conic_point_2<Alg_kernel>                      Point_2;

private:

  // Type definition for the intersection points mapping.
  typedef typename X_monotone_curve_2::Conic_id           Conic_id;
  typedef typename X_monotone_curve_2::Intersection_point_2
                                                          Intersection_point_2;
  typedef typename X_monotone_curve_2::Intersection_map   Intersection_map;

  unsigned int      conic_index; // The serial number of the last conic
                                 // curve that was handled.
  Intersection_map  inter_map;   // Mapping conic pairs to their intersection
                                 // points.

public:

  /*!
   * Default constructor.
   */
  Arr_conic_traits_2 () :
    conic_index (0)
  {}

  /// \name Functor definitions.
  //@{

  class Compare_x_2
  {
  public:
    /*!
     * Compare the x-coordinates of two points.
     * \param p1 The first point.
     * \param p2 The second point.
     * \return LARGER if x(p1) > x(p2);
     *         SMALLER if x(p1) < x(p2);
     *         EQUAL if x(p1) = x(p2).
     */
    Comparison_result operator() (const Point_2 & p1, const Point_2 & p2) const
    {
      Alg_kernel   ker;
      return (ker.compare_x_2_object() (p1, p2));
    }
  };

  /*! Get a Compare_x_2 functor object. */
  Compare_x_2 compare_x_2_object () const
  {
    return Compare_x_2();
  }

  class Compare_xy_2
  {
  public:
    /*!
     * Compares two points lexigoraphically: by x, then by y.
     * \param p1 The first point.
     * \param p2 The second point.
     * \return LARGER if x(p1) > x(p2), or if x(p1) = x(p2) and y(p1) > y(p2);
     *         SMALLER if x(p1) < x(p2), or if x(p1) = x(p2) and y(p1) < y(p2);
     *         EQUAL if the two points are equal.
     */
    Comparison_result operator() (const Point_2& p1, const Point_2& p2) const
    {
      Alg_kernel   ker;
      return (ker.compare_xy_2_object() (p1, p2));
    }
  };

  /*! Get a Compare_xy_2 functor object. */
  Compare_xy_2 compare_xy_2_object () const
  {
    return Compare_xy_2();
  }

  class Construct_min_vertex_2
  {
  public:
    /*!
     * Get the left endpoint of the x-monotone curve (segment).
     * \param cv The curve.
     * \return The left endpoint.
     */
    const Point_2& operator() (const X_monotone_curve_2 & cv) const
    {
      return (cv.left());
    }
  };

  /*! Get a Construct_min_vertex_2 functor object. */
  Construct_min_vertex_2 construct_min_vertex_2_object () const
  {
    return Construct_min_vertex_2();
  }

  class Construct_max_vertex_2
  {
  public:
    /*!
     * Get the right endpoint of the x-monotone curve (segment).
     * \param cv The curve.
     * \return The right endpoint.
     */
    const Point_2& operator() (const X_monotone_curve_2 & cv) const
    {
      return (cv.right());
    }
  };

  /*! Get a Construct_max_vertex_2 functor object. */
  Construct_max_vertex_2 construct_max_vertex_2_object () const
  {
    return Construct_max_vertex_2();
  }

  class Is_vertical_2
  {
  public:
    /*!
     * Check whether the given x-monotone curve is a vertical segment.
     * \param cv The curve.
     * \return (true) if the curve is a vertical segment; (false) otherwise.
     */
    bool operator() (const X_monotone_curve_2& cv) const
    {
      return (cv.is_vertical());
    }
  };

  /*! Get an Is_vertical_2 functor object. */
  Is_vertical_2 is_vertical_2_object () const
  {
    return Is_vertical_2();
  }

  class Compare_y_at_x_2
  {
  public:
    /*!
     * Return the location of the given point with respect to the input curve.
     * \param cv The curve.
     * \param p The point.
     * \pre p is in the x-range of cv.
     * \return SMALLER if y(p) < cv(x(p)), i.e. the point is below the curve;
     *         LARGER if y(p) > cv(x(p)), i.e. the point is above the curve;
     *         EQUAL if p lies on the curve.
     */
    Comparison_result operator() (const Point_2 & p,
                                  const X_monotone_curve_2 & cv) const
    {
      Alg_kernel   ker;

      if (cv.is_vertical())
      {
        // A special treatment for vertical segments:
        // In case p has the same x c-ordinate of the vertical segment, compare
        // it to the segment endpoints to determine its position.
        Comparison_result res1 = ker.compare_y_2_object() (p, cv.left());
        Comparison_result res2 = ker.compare_y_2_object() (p, cv.right());

        if (res1 == res2)
          return (res1);
        else
          return (EQUAL);
      }

      // Check whether the point is exactly on the curve.
      if (cv.contains_point(p))
        return (EQUAL);

      // Get a point q on the x-monotone arc with the same x coordinate as p.
      Comparison_result  x_res; 
      Point_2            q;

      if ((x_res = ker.compare_x_2_object() (p, cv.left())) == EQUAL)
      {
        q = cv.left();
      }
      else
      {
	CGAL_precondition (x_res != SMALLER);

	if ((x_res = ker.compare_x_2_object() (p, cv.right())) == EQUAL)
	{
	  q = cv.right();
	}
	else
	{
	  CGAL_precondition (x_res != LARGER);

	  q = cv.get_point_at_x (p);
	}
      }

      // Compare p with the a point of the curve with the same x coordinate.
      return (ker.compare_y_2_object() (p, q));
    }
  };

  /*! Get a Compare_y_at_x_2 functor object. */
  Compare_y_at_x_2 compare_y_at_x_2_object () const
  {
    return Compare_y_at_x_2();
  }

  class Compare_y_at_x_left_2
  {
  public:
    /*!
     * Compares the y value of two x-monotone curves immediately to the left
     * of their intersection point.
     * \param cv1 The first curve.
     * \param cv2 The second curve.
     * \param p The intersection point.
     * \pre The point p lies on both curves, and both of them must be also be
     *      defined (lexicographically) to its left.
     * \return The relative position of cv1 with respect to cv2 immdiately to
     *         the left of p: SMALLER, LARGER or EQUAL.
     */
    Comparison_result operator() (const X_monotone_curve_2& cv1,
                                  const X_monotone_curve_2& cv2,
                                  const Point_2& p) const
    {
      // Make sure that p lies on both curves, and that both are defined to its
      // left (so their left endpoint is lexicographically smaller than p).
      CGAL_precondition (cv1.contains_point (p) &&
                         cv2.contains_point (p));

      CGAL_precondition_code (
        Alg_kernel   ker;
      );
      CGAL_precondition (ker.compare_xy_2_object() (p, 
                                                    cv1.left()) == LARGER &&
                         ker.compare_xy_2_object() (p,
                                                    cv2.left()) == LARGER);

      // If one of the curves is vertical, it is below the other one.
      if (cv1.is_vertical())
      {
        if (cv2.is_vertical())
          // Both are vertical:
          return (EQUAL);
        else
          return (SMALLER);
      }
      else if (cv2.is_vertical())
      {
        return (LARGER);
      }

      // Compare the two curves immediately to the left of p:
      return (cv1.compare_to_left (cv2, p));
    }
  };

  /*! Get a Compare_y_at_x_left_2 functor object. */
  Compare_y_at_x_left_2 compare_y_at_x_left_2_object () const
  {
    return Compare_y_at_x_left_2();
  }

  class Compare_y_at_x_right_2
  {
  public:
    /*!
     * Compares the y value of two x-monotone curves immediately to the right
     * of their intersection point.
     * \param cv1 The first curve.
     * \param cv2 The second curve.
     * \param p The intersection point.
     * \pre The point p lies on both curves, and both of them must be also be
     *      defined (lexicographically) to its right.
     * \return The relative position of cv1 with respect to cv2 immdiately to
     *         the right of p: SMALLER, LARGER or EQUAL.
     */
    Comparison_result operator() (const X_monotone_curve_2& cv1,
                                  const X_monotone_curve_2& cv2,
                                  const Point_2& p) const
    {
      // Make sure that p lies on both curves, and that both are defined to its
      // left (so their left endpoint is lexicographically smaller than p).
      CGAL_precondition (cv1.contains_point (p) &&
                         cv2.contains_point (p));

      CGAL_precondition_code (
        Alg_kernel   ker;
      );
      CGAL_precondition (ker.compare_xy_2_object() (p, 
                                                    cv1.right()) == SMALLER &&
                         ker.compare_xy_2_object() (p,
                                                    cv2.right()) == SMALLER);

      // If one of the curves is vertical, it is above the other one.
      if (cv1.is_vertical())
      {
        if (cv2.is_vertical())
          // Both are vertical:
          return (EQUAL);
        else
          return (LARGER);
      }
      else if (cv2.is_vertical())
      {
        return (SMALLER);
      }

      // Compare the two curves immediately to the right of p:
      return (cv1.compare_to_right (cv2, p));
    }
  };

  /*! Get a Compare_y_at_x_right_2 functor object. */
  Compare_y_at_x_right_2 compare_y_at_x_right_2_object () const
  {
    return Compare_y_at_x_right_2();
  }

  class Equal_2
  {
  public:
    /*!
     * Check if the two x-monotone curves are the same (have the same graph).
     * \param cv1 The first curve.
     * \param cv2 The second curve.
     * \return (true) if the two curves are the same; (false) otherwise.
     */
    bool operator() (const X_monotone_curve_2& cv1,
                     const X_monotone_curve_2& cv2) const
    {
      if (&cv1 == &cv2)
        return (true);

      return (cv1.equals (cv2));
    }

    /*!
     * Check if the two points are the same.
     * \param p1 The first point.
     * \param p2 The second point.
     * \return (true) if the two point are the same; (false) otherwise.
     */
    bool operator() (const Point_2& p1, const Point_2& p2) const
    {
      if (&p1 == &p2)
        return (true);

      Alg_kernel   ker;
      return (ker.compare_xy_2_object() (p1, p2) == EQUAL);
    }
  };

  /*! Get an Equal_2 functor object. */
  Equal_2 equal_2_object () const
  {
    return Equal_2();
  }

  class Make_x_monotone_2
  {
  private:

    const Arr_conic_traits_2<Rat_kernel,
                             Alg_kernel,
                             Nt_traits>   *p_traits;
                             
    unsigned int       *p_conic_index;         // The current conic index.

  public:

    /*! Constructor. */
    Make_x_monotone_2 (const Arr_conic_traits_2<Rat_kernel,
                                                Alg_kernel,
                                                Nt_traits> *traits,
                       unsigned int *index) :
      p_traits (traits),
      p_conic_index (index)
    {}

    /*!
     * Cut the given conic curve (or conic arc) into x-monotone subcurves 
     * and insert them to the given output iterator.
     * \param cv The curve.
     * \param oi The output iterator, whose value-type is Object. The returned
     *           objects are all wrappers X_monotone_curve_2 objects.
     * \return The past-the-end iterator.
     */
    template<class OutputIterator>
    OutputIterator operator() (const Curve_2& cv, OutputIterator oi)
    {
      // Increment the serial number of the curve cv, which will serve as its
      // unique identifier.
      (*p_conic_index)++;
      
      Conic_id      conic_id (p_traits, *p_conic_index);
      
      // Find the points of vertical tangency to cv and act accordingly.
      typename Curve_2::Point_2  vtan_ps[2];
      int                        n_vtan_ps;

      n_vtan_ps = cv.vertical_tangency_points (vtan_ps);

      if (n_vtan_ps == 0)
      {    
        // In case the given curve is already x-monotone:
        *oi = make_object (X_monotone_curve_2 (cv, conic_id));
        ++oi;
        return (oi);
      }

      // Split the conic arc into x-monotone sub-curves. 
      if (cv.is_full_conic())
      {
        // Make sure we have two vertical tangency points.
        CGAL_assertion(n_vtan_ps == 2);

        // In case the curve is a full conic, split it into two x-monotone
        // arcs, one going from ps[0] to ps[1], and the other from ps[1] to
        // ps[0].
        *oi = make_object (X_monotone_curve_2 (cv, vtan_ps[0], vtan_ps[1], 
                                               conic_id));
        ++oi;
        *oi = make_object (X_monotone_curve_2 (cv, vtan_ps[1], vtan_ps[0], 
                                               conic_id));
        ++oi;
      }
      else
      {
        if (n_vtan_ps == 1)
        {
          // Split the arc into two x-monotone sub-curves: one going from the
          // arc source to ps[0], and the other from ps[0] to the target.
          *oi = make_object (X_monotone_curve_2 (cv, cv.source(), vtan_ps[0], 
                                                 conic_id));
          ++oi;
          *oi = make_object (X_monotone_curve_2 (cv, vtan_ps[0], cv.target(), 
                                                 conic_id));
          ++oi;
        }
        else
        {
          CGAL_assertion (n_vtan_ps == 2);

          // Split the arc into three x-monotone sub-curves: one going from the
          // arc source to ps[0], one from ps[0] to ps[1], and the last one
          // from ps[1] to the target.
          // Notice that ps[0] and ps[1] might switch places.
          X_monotone_curve_2    sub_curve1 (cv, cv.source(), vtan_ps[0], 
                                            conic_id);
          X_monotone_curve_2    sub_curve2 (cv, vtan_ps[0], cv.target(),
                                            conic_id);
          X_monotone_curve_2    sub_curve3;
          X_monotone_curve_2    temp;
          Point_2               vtan_p = Point_2(vtan_ps[1]);

          if (sub_curve2.contains_point (vtan_p))
          {
            temp = sub_curve2;
            temp.split (vtan_p,
                        sub_curve2, sub_curve3);
          }
          else
          {
            CGAL_assertion (sub_curve1.contains_point (vtan_p));

            // Actually we switch between ps[0] and ps[1].
            temp = sub_curve1;
            sub_curve3 = sub_curve2;
            temp.split (vtan_p,
                        sub_curve1, sub_curve2);
          }

          *oi = make_object (sub_curve1);
          ++oi;
          *oi = make_object (sub_curve2);
          ++oi;
          *oi = make_object (sub_curve3);
          ++oi;
        }
      }

      return (oi);
    }
  };

  /*! Get a Make_x_monotone_2 functor object. */
  Make_x_monotone_2 make_x_monotone_2_object ()
  {
    return (Make_x_monotone_2 (this, &conic_index));
  }

  class Split_2
  {
  public:
    /*!
     * Split a given x-monotone curve at a given point into two sub-curves.
     * \param cv The curve to split
     * \param p The split point.
     * \param c1 Output: The left resulting subcurve (p is its right endpoint).
     * \param c2 Output: The right resulting subcurve (p is its left endpoint).
     * \pre p lies on cv but is not one of its end-points.
     */
    void operator() (const X_monotone_curve_2& cv, const Point_2 & p,
                     X_monotone_curve_2& c1, X_monotone_curve_2& c2) const
    {
      cv.split (p, c1, c2);
      return;
    }
  };

  /*! Get a Split_2 functor object. */
  Split_2 split_2_object () const
  {
    return Split_2();
  }

  class Intersect_2
  {
  private:

    Intersection_map&  _inter_map;       // The map of intersection points.

  public:

    /*! Constructor. */
    Intersect_2 (Intersection_map& map) :
      _inter_map (map)
    {}

    /*!
     * Find the intersections of the two given curves and insert them to the
     * given output iterator. As two segments may itersect only once, only a
     * single will be contained in the iterator.
     * \param cv1 The first curve.
     * \param cv2 The second curve.
     * \param oi The output iterator.
     * \return The past-the-end iterator.
     */
    template<class OutputIterator>
    OutputIterator operator() (const X_monotone_curve_2& cv1,
                               const X_monotone_curve_2& cv2,
                               OutputIterator oi)
    {
      return (cv1.intersect (cv2, _inter_map, oi));
    }
  };

  /*! Get an Intersect_2 functor object. */
  Intersect_2 intersect_2_object ()
  {
    return (Intersect_2 (inter_map));
  }

  class Are_mergeable_2
  {
  public:
    /*!
     * Check whether it is possible to merge two given x-monotone curves.
     * \param cv1 The first curve.
     * \param cv2 The second curve.
     * \return (true) if the two curves are mergeable - if they are supported
     *         by the same line and share a common endpoint; (false) otherwise.
     */
    bool operator() (const X_monotone_curve_2& cv1,
                     const X_monotone_curve_2& cv2) const
    {
      return (cv1.can_merge_with (cv2));
    }
  };

  /*! Get an Are_mergeable_2 functor object. */
  Are_mergeable_2 are_mergeable_2_object () const
  {
    return Are_mergeable_2();
  }

  class Merge_2
  {
  public:
    /*!
     * Merge two given x-monotone curves into a single curve (segment).
     * \param cv1 The first curve.
     * \param cv2 The second curve.
     * \param c Output: The merged curve.
     * \pre The two curves are mergeable, that is they are supported by the
     *      same conic curve and share a common endpoint.
     */
    void operator() (const X_monotone_curve_2& cv1,
                     const X_monotone_curve_2& cv2,
                     X_monotone_curve_2& c) const
    {
      c = cv1;
      c.merge (cv2);

      return;
    }
  };

  /*! Get a Merge_2 functor object. */
  Merge_2 merge_2_object () const
  {
    return Merge_2();
  }

  ///@}
};

CGAL_END_NAMESPACE

#endif
