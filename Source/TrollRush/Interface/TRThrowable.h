// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TRThrowable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UTRThrowable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class TROLLRUSH_API ITRThrowable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void Throw();

};
