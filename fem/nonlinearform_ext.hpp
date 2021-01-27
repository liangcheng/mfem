// Copyright (c) 2010-2020, Lawrence Livermore National Security, LLC. Produced
// at the Lawrence Livermore National Laboratory. All Rights reserved. See files
// LICENSE and NOTICE for details. LLNL-CODE-806117.
//
// This file is part of the MFEM library. For more information and source code
// availability visit https://mfem.org.
//
// MFEM is free software; you can redistribute it and/or modify it under the
// terms of the BSD-3 license. We welcome feedback and contributions, see file
// CONTRIBUTING.md for details.

#ifndef NONLINEARFORM_EXT_HPP
#define NONLINEARFORM_EXT_HPP

#include "../config/config.hpp"
#include "fespace.hpp"

namespace mfem
{

class NonlinearForm;
class NonlinearFormIntegrator;

/** @brief Class extending the NonlinearForm class to support the different
    AssemblyLevel%s. */
/** This class represents the action of the NonlinearForm as an L-to-L operator,
    i.e. both the input and output Vectors are L-vectors (GridFunction-size
    vectors). Essential boundary conditions are NOT applied to the action of the
    operator. */
class NonlinearFormExtension : public Operator
{
protected:
   const NonlinearForm *nlf; ///< Not owned

public:
   NonlinearFormExtension(const NonlinearForm*);

   /// Assemble at the AssemblyLevel of the subclass.
   virtual void Assemble() = 0;

   /** @brief Return the gradient as an L-to-L Operator. The input @a x must be
       an L-vector (i.e. GridFunction-size vector). */
   /** Essential boundary conditions are NOT applied to the returned operator.

       The returned gradient Operator defines the virtual method GetProlongation
       which enables support for the method FormSystemOperator to define the
       matrix-free global true-dof gradient with imposed boundary conditions. */
   virtual Operator &GetGradient(const Vector &x) const = 0;

   /// Assumes that @a x is a ldof Vector.
   virtual double GetGridFunctionEnergy(const Vector &x) const = 0;

   /// Called by NonlinearForm::Update().
   virtual void Update() = 0;
};

/// Data and methods for partially-assembled nonlinear forms
class PANonlinearFormExtension : public NonlinearFormExtension
{
private:
   class Gradient : public Operator
   {
   protected:
      const PANonlinearFormExtension &ext;
      mutable Vector ge;

   public:
      /// Assumes that @a g is a ldof Vector.
      Gradient(const PANonlinearFormExtension &ext);

      /// Assumes that @a x and @a y are ldof Vector%s.
      virtual void Mult(const Vector &x, Vector &y) const;

      /// Assumes that @a g is an ldof Vector.
      void AssembleGrad(const Vector &g);

      /// Assemble the diagonal of the gradient into the ldof Vector @a diag.
      virtual void AssembleDiagonal(Vector &diag) const;

      /** @brief Define the prolongation Operator for use with methods like
          FormSystemOperator. */
      virtual const Operator *GetProlongation() const
      {
         return ext.fes.GetProlongationMatrix();
      }

      /** @brief Called by PANonlinearFormExtension::Update to reflect changes
          in the FiniteElementSpace. */
      void Update();
   };

protected:
   mutable Vector xe, ye;
   const FiniteElementSpace &fes;
   const Array<NonlinearFormIntegrator*> &dnfi;
   const Operator *elemR; // not owned
   mutable Gradient Grad;

public:
   PANonlinearFormExtension(const NonlinearForm *nlf);

   void Assemble() override;

   void Mult(const Vector &x, Vector &y) const override;
   Operator &GetGradient(const Vector &x) const override;
   double GetGridFunctionEnergy(const Vector &x) const override;

   void Update() override;
};
}
#endif // NONLINEARFORM_EXT_HPP
