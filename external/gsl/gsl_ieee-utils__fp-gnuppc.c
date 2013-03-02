/* ieee-utils/fp-gnuppc.c
 * 
 * Copyright (C) 1996, 1997, 1998, 1999, 2000, 2007 Brian Gough, John Fisher
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <fpu_control.h>
#include "gsl_errno.h"
#include "gsl_ieee_utils.h"


/*
 * Identical to fp-gnux86.c, except with references to
 * _FPU_SINGLE, _FPU_DOUBLE, _FPU_EXTENDED, _FPU_MASK_DM
 * and _FPU_MASK_PM converted to errors.
 */

int
gsl_ieee_set_mode (int precision, int rounding, int exception_mask)
{
  unsigned short mode = 0 ;

  switch (precision)
    {
    case GSL_IEEE_SINGLE_PRECISION:
      GSL_ERROR ("powerpc only supports default precision rounding", GSL_EUNSUP);
      break ;
    case GSL_IEEE_DOUBLE_PRECISION:
      GSL_ERROR ("powerpc only supports default precision rounding", GSL_EUNSUP);
      break ;
    case GSL_IEEE_EXTENDED_PRECISION:
      GSL_ERROR ("powerpc only supports default precision rounding", GSL_EUNSUP);
      break ;
    }

  switch (rounding)
    {
    case GSL_IEEE_ROUND_TO_NEAREST:
      mode |= _FPU_RC_NEAREST ;
      break ;
    case GSL_IEEE_ROUND_DOWN:
      mode |= _FPU_RC_DOWN ;
      break ;
    case GSL_IEEE_ROUND_UP:
      mode |= _FPU_RC_UP ;
      break ;
    case GSL_IEEE_ROUND_TO_ZERO:
      mode |= _FPU_RC_ZERO ;
      break ;
    default:
      mode |= _FPU_RC_NEAREST ;
    }

  if (exception_mask & GSL_IEEE_MASK_INVALID)
    mode |= _FPU_MASK_IM ;

  if (exception_mask & GSL_IEEE_MASK_DENORMALIZED)
    {
      /* do nothing */
    }
  else
    {
      GSL_ERROR ("powerpc does not support the denormalized operand exception. "
                 "Use 'mask-denormalized' to work around this.", GSL_EUNSUP) ;
    }

  if (exception_mask & GSL_IEEE_MASK_DIVISION_BY_ZERO)
    mode |= _FPU_MASK_ZM ;

  if (exception_mask & GSL_IEEE_MASK_OVERFLOW)
    mode |= _FPU_MASK_OM ;

  if (exception_mask & GSL_IEEE_MASK_UNDERFLOW)
    mode |= _FPU_MASK_UM ;

  if (exception_mask & GSL_IEEE_TRAP_INEXACT)
    {
     GSL_ERROR ("powerpc does not support traps for inexact operations", GSL_EUNSUP) ;
    }

  _FPU_SETCW(mode) ;

  return GSL_SUCCESS ;
}
