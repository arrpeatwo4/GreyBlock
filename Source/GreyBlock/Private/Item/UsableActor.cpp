// Fill out your copyright notice in the Description page of Project Settings.

#include "GreyBlock.h"
#include "UsableActor.h"


// Sets default values
AUsableActor::AUsableActor()
{
	PrimaryActorTick.bCanEverTick = false;
}


AUsableActor::AUsableActor(const class FObjectInitializer & ObjectInitializer)
	: Super(ObjectInitializer)
{
	MeshComp = ObjectInitializer.CreateDefaultSubobject<UStaticMeshComponent>(this, TEXT("Mesh"));
	RootComponent = MeshComp;
}


void AUsableActor::OnUsed(APawn * InstigatorPawn)
{
	// Nothing to do?
}


void AUsableActor::OnBeginFocus()
{
	MeshComp->SetRenderCustomDepth(true);
}


void AUsableActor::OnEndFocus()
{
	MeshComp->SetRenderCustomDepth(false);
}