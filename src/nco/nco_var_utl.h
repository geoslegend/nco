/* $Header: /data/zender/nco_20150216/nco/src/nco/nco_var_utl.h,v 1.2 2002-05-05 20:42:23 zender Exp $ */

/* Purpose: Variable utilities */

/* Copyright (C) 1995--2002 Charlie Zender
   This software is distributed under the terms of the GNU General Public License
   See http://www.gnu.ai.mit.edu/copyleft/gpl.html for full license text */

/* Usage:
   #include "nco_var_utl.h" *//* Variable utilities */

#ifndef NCO_VAR_UTL_H
#define NCO_VAR_UTL_H

/* Standard header files */
#include <stdio.h> /* stderr, FILE, NULL, printf */
#include <stdlib.h> /* strtod, strtol, malloc, getopt, exit */
#include <string.h> /* strcmp. . . */

/* 3rd party vendors */
#include <netcdf.h> /* netCDF definitions */
#include "nco_netcdf.h" /* netCDF3.0 wrapper functions */
#ifdef _OPENMP
#include <omp.h> /* OpenMP pragmas */
#endif /* not _OPENMP */

/* Personal headers */
#include "nco.h" /* NCO definitions */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void 
var_copy /* [fnc] Copy hyperslab variables of type var_typ from op1 to op2 */
(const nc_type var_typ, /* I [enm] netCDF type */
 const long sz, /* I [nbr] Number of elements to copy */
 const ptr_unn op1, /* I [sct] Values to copy */
 ptr_unn op2); /* O [sct] Destination to copy values to */

int /* O [enm] Return code */
var_dfl_set /* [fnc] Set defaults for each member of variable structure */
(var_sct * const var); /* I [sct] Variable strucutre to initialize to defaults */

void
var_dfn /* [fnc] Define variables and write their attributes to output file */
(const int in_id, /* I [enm] netCDF input-file ID */
 const char * const fl_out, /* I [sng] Name of output file */
 const int out_id, /* I [enm] netCDF output-file ID */
 const var_sct ** const var, /* I [sct] Variables to be defined in output file */
 const int nbr_var, /* I [nbr] Number of variables to be defined */
 const dmn_sct ** const dmn_ncl, /* I [sct] Dimensions included in output file */
 const int nbr_dmn_ncl); /* I [nbr] Number of dimensions in list */

var_sct * /* O [sct] Copy of input variable */
var_dpl /* [fnc] Duplicate input variable */
(const var_sct * const var); /* I [sct] Variable to duplicate */

var_sct * /* O [sct] Variable structure */
var_fll /* [fnc] Allocate variable structure and fill with metadata */
(const int nc_id, /* I [id] netCDF file ID */
 const int var_id, /* I [id] variable ID */
 const char * const var_nm, /* I [sng] Variable name */
 const dmn_sct ** const dim, /* I [sct] Dimensions available to variable */
 const int nbr_dim); /* I [nbr] Number of dimensions in list */

var_sct * /* O [sct] Pointer to free'd variable */
var_free /* [fnc] Free all memory associated with variable structure */
(var_sct *var); /* I [sct] Variable to free */

void
var_get /* [fnc] Allocate, retrieve variable hyperslab from disk to memory */
(const int nc_id, /* I [id] netCDF file ID */
 var_sct * const var); /* I [sct] Variable to get */

nm_id_sct * /* O [sct] List with coordinate excluded */
var_lst_crd_xcl /* [fnc] Exclude given coordinates from extraction list */
(const int nc_id, /* I [id] netCDF file ID */
 const int dmn_id, /* I [id] Dimension ID of coordinate to remove from extraction list */
 nm_id_sct *xtr_lst, /* I/O [sct] Current extraction list (destroyed) */
 int * const nbr_xtr); /* I/O [nbr] Number of variables in extraction list */

nm_id_sct * /* O [sct] Extraction list */
var_lst_ass_crd_add /* [fnc] Add coordinates associated extracted variables to extraction list */
(const int nc_id, /* I netCDF file ID */
 nm_id_sct *xtr_lst, /* I/O current extraction list (destroyed) */
 int * const nbr_xtr); /* I/O number of variables in current extraction list */

void
var_refresh /* [fnc] Update variable metadata (var ID, dmn_nbr, mss_val) */
(const int nc_id, /* I [id] netCDF input-file ID */
 var_sct * const var); /* I/O [sct] Variable to update */

void
var_srt_zero /* [fnc] Zero srt array of variable structure */
(var_sct ** const var, /* I [sct] Variables whose srt arrays will be zeroed */
 const int nbr_var); /* I [nbr] Number of structures in variable structure list */

var_val_cpy /* [fnc] Copy data of variables in list from input to output file */
(const int in_id, /* I [enm] netCDF file ID */
 const int out_id, /* I [enm] netCDF output-file ID */
 var_sct ** const var, /* I [sct] list of pointers to variable structures */
 const int nbr_var); /* I [nbr] number of structures in variable structure list */

void
nco_xrf_dmn /* [fnc] Switch pointers to dimension structures so var->dim points to var->dim->xrf */
(var_sct * const var); /* I [sct] Variable to manipulate */

void
nco_xrf_var /* [fnc] Make xrf elements of variable structures point to eachother */
(var_sct * const var, /* I/O [sct] Variable */
 var_sct * const var_dpl); /* I/O [sct] Related variable */

#ifdef __cplusplus
} /* end extern "C" */
#endif /* __cplusplus */

#endif /* NCO_VAR_UTL_H */
