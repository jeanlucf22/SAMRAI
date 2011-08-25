/*************************************************************************
 *
 * This file is part of the SAMRAI distribution.  For full copyright
 * information, see COPYRIGHT and COPYING.LESSER.
 *
 * Copyright:     (c) 1997-2011 Lawrence Livermore National Security, LLC
 * Description:   Numerical routines for single patch in linear advection ex.
 *
 ************************************************************************/

#ifndef included_MblkEulerXD
#define included_MblkEulerXD

#include "SAMRAI/SAMRAI_config.h"

#include "SAMRAI/tbox/Array.h"
#include "SAMRAI/hier/IntVector.h"
#include "SAMRAI/hier/TimeInterpolateOperator.h"
#include "SAMRAI/pdat/CellVariable.h"
#include "SAMRAI/pdat/NodeVariable.h"
#include "SAMRAI/pdat/SideVariable.h"
#include "SAMRAI/appu/BoundaryUtilityStrategy.h"
#include "SAMRAI/appu/VisItDataWriter.h"

#include <string>
using namespace std;
#define included_String

#include "MblkGeometry.h"
#include "MblkHyperbolicLevelIntegrator.h"
#include "MblkHyperbolicPatchStrategy.h"

// ----------------------------------------------------------------------

using namespace SAMRAI;

class MblkEuler:
   public tbox::Serializable,
   public MblkHyperbolicPatchStrategy,
   public appu::BoundaryUtilityStrategy
{
public:
   //
   // the constructor and destructor
   //
   MblkEuler(
      const string& object_name,
      const tbox::Dimension& dim,
      tbox::Pointer<tbox::Database> input_db,
      tbox::Pointer<hier::GridGeometry>& grid_geom);

   ~MblkEuler();

   //
   // register with the framework
   //
   void
   registerModelVariables(
      MblkHyperbolicLevelIntegrator* integrator);

   //
   // set the patch initial conditions
   //
   void
   initializeDataOnPatch(
      hier::Patch& patch,
      const double data_time,
      const bool initial_time);

   //
   // Compute the stable time increment for patch using a CFL
   // condition and return the computed dt.
   //
   double
   computeStableDtOnPatch(
      hier::Patch& patch,
      const bool initial_time,
      const double dt_time);

   //
   // compute the state extrema for debugging
   //
   void
   testPatchExtrema(
      hier::Patch& patch,
      const char* pos);

   //
   // compute the fluxes and the initial update in a timestep
   //
   void
   computeFluxesOnPatch(
      hier::Patch& patch,
      const double time,
      const double dt);

   //
   // update the state (currently only for refluxing)
   //
   void
   conservativeDifferenceOnPatch(
      hier::Patch& patch,
      const double time,
      const double dt,
      bool at_syncronization);

   //
   // Tag cells for refinement using gradient detector.
   //
   void
   tagGradientDetectorCells(
      hier::Patch& patch,
      const double regrid_time,
      const bool initial_error,
      const int tag_indexindx,
      const bool uses_richardson_extrapolation_too);

   //
   //  The following routines:
   //
   //      postprocessRefine()
   //      setPhysicalBoundaryConditions()
   //
   //  are concrete implementations of functions declared in the
   //  RefinePatchStrategy abstract base class.
   //

   //
   // mark the zones to track what zones are being filled
   //
   void
   markPhysicalBoundaryConditions(
      hier::Patch& patch,
      const hier::IntVector& ghost_width_to_fill);

   //
   // set the data in the physical ghost zones
   //
   void
   setPhysicalBoundaryConditions(
      hier::Patch& patch,
      const double fill_time,
      const hier::IntVector&
      ghost_width_to_fill);

   //
   // Refine operations for cell data.
   //
   void
   preprocessRefine(
      hier::Patch& fine,
      const hier::Patch& coarse,
      const hier::Box& fine_box,
      const hier::IntVector& ratio);

   void
   postprocessRefine(
      hier::Patch& fine,
      const hier::Patch& coarse,
      const hier::Box& fine_box,
      const hier::IntVector& ratio);

   //
   // Coarsen operations for cell data.
   //
   void
   preprocessCoarsen(
      hier::Patch& coarse,
      const hier::Patch& fine,
      const hier::Box& coarse_box,
      const hier::IntVector& ratio);

   void
   postprocessCoarsen(
      hier::Patch& coarse,
      const hier::Patch& fine,
      const hier::Box& coarse_box,
      const hier::IntVector& ratio);

   /**
    * Fill the singularity conditions for the multi-block case
    */
   void
   fillSingularityBoundaryConditions(
      hier::Patch& patch,
      const hier::PatchLevel& encon_level,
      const hier::Connector& dst_to_encon,
      const double fill_time,
      const hier::Box& fill_box,
      const hier::BoundaryBox& boundary_box,
      const tbox::Pointer<hier::GridGeometry>& grid_geometry);

   /**
    * Build mapped grid on patch
    */
   void
   setMappedGridOnPatch(
      const hier::Patch& patch);

   //
   // build the volume on a mapped grid
   //
   void
   setVolumeOnPatch(
      const hier::Patch& patch);

   /**
    * Write state of MblkEuler object to the given database for restart.
    *
    * This routine is a concrete implementation of the function
    * declared in the tbox::Serializable abstract base class.
    */
   void
   putToDatabase(
      tbox::Pointer<tbox::Database> db);

   hier::IntVector
   getMultiblockRefineOpStencilWidth() const;
   hier::IntVector
   getMultiblockCoarsenOpStencilWidth();

#ifdef HAVE_HDF5
   /**
    * Register a VisIt data writer so this class will write
    * plot files that may be postprocessed with the VisIt
    * visualization tool.
    */
   void
   registerVisItDataWriter(
      tbox::Pointer<appu::VisItDataWriter> viz_writer);
#endif

   /**
    * Print all data members for MblkEuler class.
    */
   void
   printClassData(
      ostream& os) const;

private:
   /*
    * These private member functions read data from input and restart.
    * When beginning a run from a restart file, all data members are read
    * from the restart file.  If the boolean flag is true when reading
    * from input, some restart values may be overridden by those in the
    * input file.
    *
    * An assertion results if the database pointer is null.
    */
   void
   getFromInput(
      tbox::Pointer<tbox::Database> db,
      bool is_from_restart);

   void
   getFromRestart();

   /*
    * Private member function to check correctness of boundary data.
    */
   void
   checkBoundaryData(
      int btype,
      const hier::Patch& patch,
      const hier::IntVector& ghost_width_to_fill,
      const tbox::Array<int>& scalar_bconds) const;

   /*
    * The object name is used for error/warning reporting and also as a
    * string label for restart database entries.
    */
   string d_object_name;

   const tbox::Dimension d_dim;

   /*
    * We cache pointers to the grid geometry and Vizamrai data writer
    * object to set up initial data, set physical boundary conditions,
    * and register plot variables.
    */
   tbox::Pointer<hier::GridGeometry> d_grid_geometry;
#ifdef HAVE_HDF5
   tbox::Pointer<appu::VisItDataWriter> d_visit_writer;
#endif

   //
   // Data items used for nonuniform load balance, if used.
   //
   tbox::Pointer<pdat::CellVariable<double> > d_workload_variable;
   int d_workload_data_id;
   bool d_use_nonuniform_workload;

   //
   // =========================== State and Variable definitions (private) ============================
   //

   //
   // tbox::Pointer to state variable vector - [state]
   //
   int d_nState;  // depth of the state vector
   tbox::Pointer<pdat::CellVariable<double> > d_state;

   //
   // tbox::Pointer to cell volume - [v]
   //
   tbox::Pointer<pdat::CellVariable<double> > d_vol;

   //
   // tbox::Pointer to flux variable vector  - [F]
   //
   tbox::Pointer<pdat::SideVariable<double> > d_flux;

   //
   // tbox::Pointer to grid - [xyz]
   //
   tbox::Pointer<pdat::NodeVariable<double> > d_xyz;
   int d_xyz_id;

   //
   // ======================================= Initial Conditions (private) ============================
   //

   /// center of the sphere or revolution origin
   double d_center[tbox::Dimension::MAXIMUM_DIMENSION_VALUE];

   /// revolution axis
   double d_axis[tbox::Dimension::MAXIMUM_DIMENSION_VALUE];

   /// revolution radius and pos on axis of radius
   tbox::Array<tbox::Array<double> > d_rev_rad;
   tbox::Array<tbox::Array<double> > d_rev_axis;

   ///
   /// Rayleigh Taylor Shock tube experiments
   ///
   double d_dt_ampl;
   tbox::Array<double> d_amn;
   tbox::Array<double> d_m_mode;
   tbox::Array<double> d_n_mode;
   tbox::Array<double> d_phiy;
   tbox::Array<double> d_phiz;

   ///
   /// input for all the geometries
   ///

   //
   // linear advection velocity vector for unit test
   //
   int d_advection_test;      // run the linear advection unit test
   int d_advection_vel_type;  // type of velocity to use
   double d_advection_velocity[tbox::Dimension::MAXIMUM_DIMENSION_VALUE];

   //
   // sizing of zonal, flux, and nodal ghosts
   //
   hier::IntVector d_nghosts;
   hier::IntVector d_fluxghosts;
   hier::IntVector d_nodeghosts;

   //
   // Indicator for problem type and initial conditions
   //
   string d_data_problem;

   //
   // region initialization inputs
   //
   int d_number_of_regions;
   tbox::Array<double> d_front_position;

   //
   // array of initial conditions and their names [region][state]
   //
   tbox::Array<tbox::Array<double> > d_state_ic;
   tbox::Array<string> d_state_names;

   //
   // This class stores geometry information used for constructing the
   // mapped multiblock hierarchy
   //
   MblkGeometry* d_mblk_geometry;

   /// the bound on the index space for the current block
   int d_dom_current_bounds[6];

   /// the number of boxes needed to describe the index space for the current block
   int d_dom_current_nboxes;

   /// the blocks bounding the current patch
   int d_dom_local_blocks[6];

   //
   // ======================================= Refinement Data (private) ============================
   //

   tbox::Array<string> d_refinement_criteria;

   /// history variable gradient tagging tolerance
   tbox::Array<tbox::Array<double> > d_state_grad_tol;
   tbox::Array<string> d_state_grad_names;
   tbox::Array<int> d_state_grad_id;

   //
   // ======================================= Boundary Conditions (private) ============================
   //

   /// factors for the boundary conditions
   tbox::Array<int> d_wall_factors;

   //
   // Operators to be used with BlockGridGeometry
   //
   tbox::Pointer<hier::TimeInterpolateOperator> d_cell_time_interp_op;
};

#endif
