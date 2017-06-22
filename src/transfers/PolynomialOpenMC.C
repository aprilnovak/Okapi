#include "PolynomialOpenMC.h"
#include "ExtraFunctions.h"
#include "OpenMCInterface.h"

#include "MooseTypes.h"
#include "MooseVariableScalar.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "SystemBase.h"
#include "SubProblem.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

template<>
InputParameters validParams<PolynomialOpenMC>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  /* To MultiApp (MOOSE -> OkapiApp):
     source_variable = SCALAR variables holding expansion coefficients. 
                       NOTE: the order of the variables is important here, 
                       since they will be stored in the same order in the
                       OpenMC expansion_coeffs global array, and hence will
                       be interpreted as the Zernike coefficients for a given
                       Legendre order, in order of increasing Legendre order.
     to_aux_scalar = dummy variable (OpenMC is only wrapped as a MOOSE App,
                     there's no FEM-based variable to store anything in */

  /* From MultiApp (OkapiApp -> MOOSE):
     source_varibale = dummy variable
     to_aux_scalar = expansion coefficients used in MOOSE */
 
  /* Because we're transferring more than one scalar variable at a time 
     to/from Okapi, we need to allow these parameters to be a vector of names. */
  params.addRequiredParam<std::vector<VariableName>>("source_variable", \
    "The auxiliary SCALAR variable to read values from");
  params.addRequiredParam<std::vector<VariableName> >("to_aux_scalar", \
    "The name of the SCALAR auxvariable in the MultiApp to transfer the value to.");

  /* This transfer is also associated with a radius, center, and l_geom_norm that
     will be used in OpenMC to determine which cells this transfer maps to. */
  params.addParam<Real>("radius","Radius of region with Zernike polynomials.");  
  params.addParam<std::vector<Real>>("center", "center coordinates (x, y) of circle");
  params.addParam<std::vector<Real>>("l_geom_norm", "min and max coordinates\
    along the Legendre direction.");
  params.addParam<int>("l_direction", "direction of integration for Legendre.");
  return params;
}

PolynomialOpenMC::PolynomialOpenMC(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _source_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_aux_names(getParam<std::vector<VariableName>>("to_aux_scalar")),
    _radius(parameters.get<Real>("radius")),
    _center(parameters.get<std::vector<Real>>("center")),
    _geom_norm(parameters.get<std::vector<Real>>("l_geom_norm")),
    _l_direction(parameters.get<int>("l_direction"))
{
}

void
PolynomialOpenMC::execute()
{
  _console << "Beginning PolynomialOpenMC Transfer " << name() << std::endl;
  
  /* This returns the total number of _sub_ Apps, which is equal to the number
     of (x,y,z) coordinate pairs in the input file. Each of these positions
     represents an offset of a sub-application in the space of the master App
     (the local Master app). */
  unsigned int num_apps = _multi_app->numGlobalApps();

  /* Store the legendre and zernike expansion orders to pass them into OpenMC.
     A check is made to ensure that the number of Zernike coefficients is the 
     same for every coupled scalar auxvariable. */
  int legendre_order_from_MOOSE = 0;
  int zernike_order_from_MOOSE = 0;
  int old_zernike_order = 0;

/*  OpenMC::CouplingGeom geometry;
  geometry.radius = _radius;
  geometry.height = {_geom_norm[0], _geom_norm[1]};
  geometry.center = {_center[0], _center[1]};
  geometry.l_dir = _l_direction;*/
  
  switch (_direction)
  {
    /* MasterApp -> SubApp. For MOOSE-OpenMC coupling, this represents the transfer
       of information from MOOSE to OpenMC. This transfers _all_ of the expansion 
       coefficients, but one at a time. */
    case TO_MULTIAPP:
    {
      /* The MultiAppTransfer class defines a parameter _multi_app which gets the 
         _fe_problem for the MultiApp according to the "multi_app = OkapiApp" 
         provided in the input file. This represents the MasterApp problem, and is 
         from where we will retrieve the values of the source_variables. */
      FEProblemBase & from_problem = _multi_app->problemBase();

      /* Create a vector of pointers that will point to the source_variables (of 
         MooseVariableScalar type) and is of the same length as the number of 
         source_varibales. */
      std::vector<MooseVariableScalar *> source_variables(_source_var_names.size());
      for (auto i = beginIndex(_source_var_names); i < _source_var_names.size(); ++i)
      {
        source_variables[i] = &from_problem.getScalarVariable(_tid,\
          _source_var_names[i]);
        source_variables[i]->reinit(); //?
      }

      /* Loop through each of the sub apps. Because MC neutron transport simulations 
         require knowledge of the entire domain (i.e. cannot specify something like
         a temperature boundary condition in a Monte Carlo code), we'll likely never
         have multiple OpenMC sub apps. However, this would be relevant for BISON
         applications, where each fuel pin is an independent solve. */
      for (unsigned int i = 0; i < num_apps; i++)
        if (_multi_app->hasLocalApp(i)) // if global app number is on this processor
        { 
          // Loop through each SCALAR variable. The order of the Legendre expansion
          // equals the number of source variables - 1, since the Legendre order
          // can be zero.
          legendre_order_from_MOOSE = _source_var_names.size() - 1;
          for (auto i = beginIndex(_source_var_names); i < _source_var_names.size(); ++i)
          { 
            // Loop through each entry in a single SCALAR variable
            auto & solution_values = source_variables[i]->sln();

            // Check that Zernike order is the same for every coupled scalar auxvar
            old_zernike_order = zernike_order_from_MOOSE;

            if (i > 0)
              if (old_zernike_order != zernike_order_from_MOOSE)
              {
                mooseError("The order of the Zernike expansion does not match each \
                  coupling Legendre coefficient aux variable.");
              }

            zernike_order_from_MOOSE = zernike_order_from_coeffs(solution_values.size());
            for (auto j = beginIndex(solution_values); j < solution_values.size(); ++j)
            {
              OpenMC::receive_coeffs(solution_values[j]);
            }
            _console << '\n';
          }
        
        // Transfer the order of the Zernike and Legendre expansions to OpenMC
        // and print the expansion coefficients received.
        OpenMC::receive_coupling_info(legendre_order_from_MOOSE, zernike_order_from_MOOSE);
        }

      /* Once you have read the data into OpenMC data structures, use it to perform somce
         action in OpenMC, such as using expansion coefficients to reconstruct a 
         continuous field. */
   //   FORTRAN_CALL(Nek5000::flux_reconstruction)();
      OpenMC::change_fuel_temp();
      break;
    }

    // SubApp -> MasterApp
    case FROM_MULTIAPP:
    {
      /* Before transferring any data back up to the Master App (MOOSE), OpenMC will need 
         to expand the continuous transfer variable into expansion coefficients. Then,
         we will store these expansion coefficients in the same type of data structure
         that is used to transfer from MOOSE to OpenMC. */
     // FORTRAN_CALL(Nek5000::nek_expansion)();
      OpenMC::create_coeffs();
      std::vector<MooseVariableScalar *> to_variables(_to_aux_names.size());
      for (auto i = beginIndex(_to_aux_names); i < _to_aux_names.size(); ++i)
      {
        to_variables[i] = &_multi_app->problemBase().getScalarVariable(_tid, _to_aux_names[i]);
        to_variables[i]->reinit();
      }

      // Loop over the variables that we are going to write
      for (auto i = beginIndex(_to_aux_names); i < _to_aux_names.size(); ++i)
      {
        std::vector<dof_id_type> & dof = to_variables[i]->dofIndices();
        auto & solution_values = to_variables[i]->sln();
        for (auto j = beginIndex(solution_values); j < solution_values.size(); ++j)
        {
          to_variables[i]->sys().solution().set(dof[j], 600.0);
          to_variables[i]->sys().solution().close();
        }
      }

      break;
    }
  }

  _console << "Finished PolynomialOpenMC transfer" << name() << std::endl;
}
