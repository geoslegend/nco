/* $Header: /data/zender/nco_20150216/nco/src/nco/nco_var_scv.h,v 1.3 2002-05-07 08:34:15 zender Exp $ */

/* Purpose: Arithmetic between variables and scalar values */

/* Copyright (C) 1995--2002 Charlie Zender
   This software is distributed under the terms of the GNU General Public License
   See http://www.gnu.ai.mit.edu/copyleft/gpl.html for full license text */

/* Usage:
   #include "nco_var_scv.h" *//* Arithmetic between variables and scalar values */

#ifndef NCO_VAR_SCV_H
#define NCO_VAR_SCV_H

/* Standard header files */
#include <math.h> /* sin cos cos sin 3.14159 */
#include <stdio.h> /* stderr, FILE, NULL, printf */

/* 3rd party vendors */
#include <netcdf.h> /* netCDF definitions */
#include "nco_netcdf.h" /* netCDF3.0 wrapper functions */

/* Personal headers */
#include "nco.h" /* NCO definitions */
#include "nco_cnf_typ.h" /* Conform variable types */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  void var_scv_add(nc_type,const long,int,ptr_unn,ptr_unn,scv_sct *);
  void var_scv_sub(nc_type,const long,int,ptr_unn,ptr_unn,scv_sct *);
  void var_scv_divide(nc_type,const long,int,ptr_unn,ptr_unn,scv_sct *);
  void var_scv_modulus(nc_type,const long,int,ptr_unn,ptr_unn,scv_sct *);
  void var_scv_multiply(nc_type,const long,int,ptr_unn,ptr_unn,scv_sct *);

#ifdef __cplusplus
} /* end extern "C" */
#endif /* __cplusplus */

#endif /* NCO_VAR_SCV_H */
