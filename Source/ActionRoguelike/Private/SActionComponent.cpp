// Fill out your copyright notice in the Description page of Project Settings.


#include "SActionComponent.h"
#include "SAction.h"


// Sets default values for this component's properties
USActionComponent::USActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}


void USActionComponent::BeginPlay()
{
	Super::BeginPlay();

	for(TSubclassOf<USAction> ActionClass : DefaultActions)
	{
		AddAction(GetOwner(),ActionClass);
	}
}


void USActionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Display active tags
	GEngine->AddOnScreenDebugMessage(-1,0.0f,FColor::White,GetNameSafe(GetOwner()) + " : " + ActiveGamePlayTags.ToStringSimple());
}


void USActionComponent::AddAction(AActor* Instigator,TSubclassOf<USAction> ActionClass)
{
	if(!ensure(ActionClass))
	{
		return;
	}
	USAction* NewAction= NewObject<USAction>(this,ActionClass);
	if(ensure(NewAction))
	{
		Actions.Add(NewAction);

		if(NewAction->bAutoStart && ensure(NewAction->CanStart(Instigator)))
		{
			NewAction->StartAction(Instigator);
		}
	}
}



bool USActionComponent::StartActionByName(AActor* Instigator, FName ActionName)
{
	for (USAction* Action : Actions)
	{
		if(Action && Action->ActionName == ActionName)
		{
			if(!Action->CanStart(Instigator))
			{
				FString FailedMsg= FString::Printf(TEXT("Failed to run: %s"),*ActionName.ToString());
				GEngine->AddOnScreenDebugMessage(-1,2.0f,FColor::Red,FailedMsg);
				continue;
			}
			if(!GetOwner()->HasAuthority()) //is client?
			{
				ServerStartAction(Instigator,ActionName);
			}
			
			Action->StartAction(Instigator);
			return true;
		}
	
	}
	return false;
}

bool USActionComponent::StopActionByName(AActor* Instigator, FName ActionName)
{
	for (USAction* Action : Actions)
	{
		if(Action && Action->ActionName == ActionName)
		{
			if(Action->IsRunning())
			{
				Action->StopAction(Instigator);
				return true;
			}

		}
		
	}
	return false;
}


void USActionComponent::RemoveAction(USAction* ActionToRemove)
{
	if(!ensure(ActionToRemove && !ActionToRemove->IsRunning()))
	{
		return;
	}

	Actions.Remove(ActionToRemove);
}

void USActionComponent::ServerStartAction_Implementation(AActor* Instigator, FName ActionName)
{
 StartActionByName(Instigator,ActionName);	
}