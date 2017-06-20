#include "PolynomialOpenMC.h"

#include "MooseTypes.h"
#include "MooseVariableScalar.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "SubProblem.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

#include "OpenMCInterface.h"

template<>
InputParameters validParams<PolynomialOpenMC>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  /* To MultiApp (MOOSE -> OkapiApp):
     source_variable = SCALAR variables holding expansion coefficients
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
  return params;
}

PolynomialOpenMC::PolynomialOpenMC(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _source_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_aux_names(getParam<std::vector<VariableName>>("to_aux_scalar"))
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
        source_variables[i] = &from_problem.getScalarVariable(_tid, _source_var_names[i]);
        source_variables[i]->reinit(); //?
      }

      /* Loop through each of the sub apps. Because MC neutron transport simulations 
         require knowledge of the entire domain (i.e. you cannot specify something like
         a temperature boundary condition in a Monte Carlo code), we'll likely not ever
         have multiple OpenMC sub apps. However, this would be relevant for BISON
         applications, where we would simulate each fuel pin as an independent solve. */
      for (unsigned int i = 0; i < num_apps; i++)
        if (_multi_app->hasLocalApp(i)) // if the global app number is on this processor
        { 
          // Loop through each SCALAR variables
          for (auto i = beginIndex(_source_var_names); i < _source_var_names.size(); ++i)
          { 
            // Loop through each entry in a single SCALAR variable
            auto & solution_values = source_variables[i]->sln();
            for (auto j = beginIndex(solution_values); j < solution_values.size(); ++j)
            {
              //OpenMC::change_fuel_temp(solution_values[j]);  
              //OpenMC::change_batches(solution_values[j]);
//              OpenMC::receive_coeffs(solution_values[j]);
            }
            _console << '\n';
          }
        }

      /* Once you have read the data into OpenMC data structures, use it to perform somce
         action in OpenMC, such as using expansion coefficients to reconstruct a 
         continuous field. */
   //   FORTRAN_CALL(Nek5000::flux_reconstruction)();
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

      std::vector<MooseVariableScalar *> to_variables(_to_aux_names.size());
      for (auto i = beginIndex(_to_aux_names); i < _to_aux_names.size(); ++i)
      {
        to_variables[i] = &_multi_app->problemBase().getScalarVariable(_tid, _to_aux_names[i]);
        to_variables[i]->reinit();
      }

      for (auto i = beginIndex(_to_aux_names); i < _to_aux_names.size(); ++i)
      {
        // The dof indices for the scalar variable of interest
        std::vector<dof_id_type> & dof = to_variables[i]->dofIndices();

//      // Error if there is a size mismatch between the scalar AuxVariable and the number of sub apps
//      if (num_apps != scalar.sln().size())
//        mooseError("The number of sub apps (" << num_apps << ") must be equal to the order of the scalar AuxVariable (" << scalar.order() << ")");

        // Loop over each sub-app and populate the AuxVariable values from the postprocessors
//        for (unsigned int i=0; i<_multi_app->numGlobalApps(); i++)

//        if (_multi_app->hasLocalApp(i) && _multi_app->isRootProcessor())
          // Note: This can't be done using MooseScalarVariable::insert() because different processors will be setting dofs separately.
/*        auto & solution_values = to_variables[i]->sln();
        for (auto j = beginIndex(solution_values); j < solution_values.size(); ++j)
        {
       //   to_variables[i]->sys().solution().set(dof[j], Nek5000::expansion_tcoef_.coeff_tij[i*100+j]);
          //to_variables[i]->sys().solution().close();
        }*/
      }

      break;
    }
  }

  _console << "Finished PolynomialToNekTransfer" << name() << std::endl;
}
