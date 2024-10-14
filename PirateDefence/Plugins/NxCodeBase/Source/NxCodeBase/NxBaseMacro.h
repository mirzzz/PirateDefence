// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


//--------------------------------------------------------------------------------
// Base new/delete
//--------------------------------------------------------------------------------
#ifndef Nx_SAFE_DELETE
#define Nx_SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=nullptr; } }
#define Nx_SAFE_DELETEARRAY(p)		{ if(p) { delete [] (p);  (p)=nullptr; } }
#define Nx_SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }
#endif


//--------------------------------------------------------------------------------
// Base Bit Flag
//--------------------------------------------------------------------------------
// - (Mask,Mask)

#ifndef Nx_BIT_SET
#define Nx_BIT_SET(mask, value)			(mask |= value)
#define Nx_BIT_REMOVE(mask, value)		(mask &= ~value)
#define Nx_BIT_TOGGLE(mask, value)		(mask ^= value)
#define Nx_BIT_CHECK(mask, value)		((mask & value) != 0)
#define Nx_BIT_CHECK_uint8(mask, value)	(((uint8)mask & (uint8)value) != 0)


// - (Mask, Shift)
#define Nx_BITSHIFT_SET(mask, shift)	(mask |= (1<<static_cast<uint32>(shift)))
#define Nx_BITSHIFT_CLEAR(mask, shift)	(mask &= ~(1<<static_cast<uint32>(shift)))
#define Nx_BITSHIFT_TOGGLE(mask, shift)	(mask ^= (1<<static_cast<uint32>(shift)))
#define Nx_BITSHIFT_CHECK(mask, shift)	((mask & (1<<static_cast<uint32>(shift))) != 0)
#endif

