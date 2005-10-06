// Copyright (c) 2005  INRIA (France).
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
// $Revision$
// $Name$
//
// Author(s)     : Laurent Saboret, Pierre Alliez


#ifndef CGAL_DISCRETE_CONFORMAL_MAP_PARAMETIZER_3_H
#define CGAL_DISCRETE_CONFORMAL_MAP_PARAMETIZER_3_H

#include <CGAL/Fixed_border_parametizer_3.h>
#include <CGAL/parameterization_assertions.h>

CGAL_BEGIN_NAMESPACE


/// Class Discrete_conformal_map_parametizer_3
/// implements Discrete Conformap Map parameterization.
/// 1 to 1 mapping is guaranteed if surface's border is mapped onto a convex polygon.
/// This is a conformal parameterization, i.e. it attempts to preserve angles.
///
/// The superclass Fixed_border_parametizer_3 implements most of the parameterization algorithm.
/// Discrete_conformal_map_parametizer_3:
/// - provides default BorderParametizer_3 and SparseLinearAlgebraTraits_d template
///   parameters that make sense.
/// - implements compute_wij() to compute wij = (i,j) coefficient of matrix A
///   for j neighbor vertex of i based on Discrete Conformap Map method.
///
/// Concept: Model of the ParametizerTraits_3 concept.
///
/// Design pattern:
/// ParametizerTraits_3 models are Strategies (see [GOF95]): they implement
/// a strategy of surface parameterization for models of MeshAdaptor_3.

template
<
    class MeshAdaptor_3,              ///< 3D surface mesh
    class BorderParametizer_3         ///< Strategy to parameterize the surface border
                = Circular_border_arc_length_parametizer_3<MeshAdaptor_3>,
    class SparseLinearAlgebraTraits_d ///< Traits class to solve a sparse linear system
                = OpenNL::DefaultLinearSolverTraits<typename MeshAdaptor_3::NT>
                                      ///< Note: the sparse linear system is symmetric
                                      ///< (except around holes) iff
                                      ///< Fixed_border_parametizer_3 removes fixed vertices
>
class Discrete_conformal_map_parametizer_3
    : public Fixed_border_parametizer_3<MeshAdaptor_3,
                                        BorderParametizer_3,
                                        SparseLinearAlgebraTraits_d>
{
// Private types
private:

    /// Superclass
    typedef Fixed_border_parametizer_3<MeshAdaptor_3,
                                        BorderParametizer_3,
                                        SparseLinearAlgebraTraits_d>
                                            Base;

// Public types
public:
    // We have to repeat the types exported by superclass
    /// @cond SKIP_IN_MANUAL
    typedef typename Base::Error_code       Error_code;
    typedef MeshAdaptor_3                   Adaptor;
    typedef typename Adaptor::NT            NT;
    typedef typename Adaptor::Point_2       Point_2;
    typedef typename Adaptor::Point_3       Point_3;
    typedef typename Adaptor::Vector_2      Vector_2;
    typedef typename Adaptor::Vector_3      Vector_3;
    typedef typename Adaptor::Facet         Facet;
    typedef typename Adaptor::Facet_handle  Facet_handle;
    typedef typename Adaptor::Facet_const_handle
                                            Facet_const_handle;
    typedef typename Adaptor::Facet_iterator Facet_iterator;
    typedef typename Adaptor::Facet_const_iterator
                                            Facet_const_iterator;
    typedef typename Adaptor::Vertex        Vertex;
    typedef typename Adaptor::Vertex_handle Vertex_handle;
    typedef typename Adaptor::Vertex_const_handle
                                            Vertex_const_handle;
    typedef typename Adaptor::Vertex_iterator Vertex_iterator;
    typedef typename Adaptor::Vertex_const_iterator
                                            Vertex_const_iterator;
    typedef typename Adaptor::Border_vertex_iterator
                                            Border_vertex_iterator;
    typedef typename Adaptor::Border_vertex_const_iterator
                                            Border_vertex_const_iterator;
    typedef typename Adaptor::Vertex_around_facet_circulator
                                            Vertex_around_facet_circulator;
    typedef typename Adaptor::Vertex_around_facet_const_circulator
                                            Vertex_around_facet_const_circulator;
    typedef typename Adaptor::Vertex_around_vertex_circulator
                                            Vertex_around_vertex_circulator;
    typedef typename Adaptor::Vertex_around_vertex_const_circulator
                                            Vertex_around_vertex_const_circulator;
    typedef BorderParametizer_3             Border_param;
    typedef SparseLinearAlgebraTraits_d     Sparse_LA;
    typedef typename Sparse_LA::Vector      Vector;
    typedef typename Sparse_LA::Matrix      Matrix;
    /// @endcond

// Public operations
public:
    /// Constructor
    Discrete_conformal_map_parametizer_3(Border_param border_param = Border_param(),
                                            ///< Object that maps the surface's border to 2D space
                                         Sparse_LA sparse_la = Sparse_LA())
                                            ///< Traits object to access a sparse linear system
    :   Fixed_border_parametizer_3<Adaptor,
                                   Border_param,
                                   Sparse_LA>(border_param, sparse_la)
    {}

    // Default copy constructor and operator =() are fine

// Protected types
protected:
    typedef typename OpenNL::LinearSolver<Sparse_LA>
                                            Solver ;

// Protected operations
protected:
    /// compute wij = (i,j) coefficient of matrix A for j neighbor vertex of i
    virtual NT  compute_wij(const Adaptor& mesh,
                            Vertex_const_handle main_vertex_Vi,
                            Vertex_around_vertex_const_circulator neighbor_vertex_Vj)
    {
        Point_3 position_Vi = mesh.get_vertex_position(main_vertex_Vi);
        Point_3 position_Vj = mesh.get_vertex_position(neighbor_vertex_Vj);

        // Compute cotangent of (Vi,Vk,Vj) corner (ie cotan of Vk corner)
        // if Vk is the vertex before Vj when circulating around Vi
        Vertex_around_vertex_const_circulator previous_vertex_Vk = neighbor_vertex_Vj;
        previous_vertex_Vk --;
        Point_3 position_Vk = mesh.get_vertex_position(previous_vertex_Vk);
        double cotg_beta_ij  = cotangent(position_Vi, position_Vk, position_Vj);

        // Compute cotangent of (Vj,Vl,Vi) corner (ie cotan of Vl corner)
        // if Vl is the vertex after Vj when circulating around Vi
        Vertex_around_vertex_const_circulator next_vertex_Vl = neighbor_vertex_Vj;
        next_vertex_Vl ++;
        Point_3 position_Vl = mesh.get_vertex_position(next_vertex_Vl);
        double cotg_alpha_ij = cotangent(position_Vj, position_Vl, position_Vi);

        double weight = cotg_beta_ij+cotg_alpha_ij;
        return weight;
    }
};


CGAL_END_NAMESPACE

#endif //CGAL_DISCRETE_CONFORMAL_MAP_PARAMETIZER_3_H

