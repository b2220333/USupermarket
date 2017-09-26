#define TAG_KEY_HOOK "Hook"
#define TAG_HOOK "Refill;Hook,True;"

#include "Refill.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "HoleTabComponent.h"
#include "TagStatics.h"
#include "ItemManager.h"


// Sets default values
AItemManager::AItemManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AItemManager::BeginPlay()
{
	Super::BeginPlay();

	// Get all hooks in the world
	SetOfHooks = FTagStatics::GetComponentSetWithKeyValuePair(GetWorld(), FString("Refill"), TAG_KEY_HOOK, "True");
	UE_LOG(LogTemp, Log, TEXT("%s: Hooks found %i"), *FString(__FUNCTION__), SetOfHooks.Num());

	// Read all the Hook names
	for (auto& HookComponent : SetOfHooks) {
		HooknamesToHookComponent.Add(HookComponent->GetFName(), HookComponent);
	}
}

// Called every frame
void AItemManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AItemManager::AddItemToHook(UPrimitiveComponent* HookComponent, ARRefillObject * ItemToHookUp)
{
	if (HookComponent == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("%s: Trying to place an item on a hook but hook was not found"), *FString(__FUNCTION__));
	}
	else {
		// Create a physics contraint
		UPhysicsConstraintComponent* ConstraintComponent = NewObject<UPhysicsConstraintComponent>(ItemToHookUp);

		if (ConstraintComponent == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("%s: Could not create constraint component on %s"), *FString(__FUNCTION__), *ItemToHookUp->GetName());
			return;
		}

		// Get the Hole component of the item
		UPrimitiveComponent* HoleTabComponent = Cast<UPrimitiveComponent>(ItemToHookUp->GetComponentByClass(UHoleTabComponent::StaticClass()));

		if (HoleTabComponent == nullptr) {
			UE_LOG(LogTemp, Warning, TEXT("%s: HoleTab Component not found on actor %s"), *FString(__FUNCTION__), *ItemToHookUp->GetName());
			return;
		}

		// Setup the constraint
		UPrimitiveComponent* ItemStaticMeshRootComponent = Cast<UPrimitiveComponent>(ItemToHookUp->GetRootComponent());

		ConstraintComponent->SetWorldLocation(HoleTabComponent->GetComponentLocation());

		ConstraintComponent->AttachToComponent(ItemToHookUp->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
		ConstraintComponent->SetConstrainedComponents(ItemStaticMeshRootComponent, NAME_None, HookComponent, NAME_None);
		// *** *** ***

		// Physics and Collisions
		ConstraintComponent->SetDisableCollision(true);
		ConstraintComponent->SetAngularSwing1Limit(EAngularConstraintMotion::ACM_Locked, 0.0f);
		ConstraintComponent->SetAngularSwing2Limit(EAngularConstraintMotion::ACM_Limited, 5.0f); // TODO Hardcoded limits
		ConstraintComponent->SetAngularTwistLimit(EAngularConstraintMotion::ACM_Limited, 5.0f);
		// *** *** ***

		UE_LOG(LogTemp, Log, TEXT("%s: Added constraint"), *FString(__FUNCTION__));


		// Add it to the map
		if (ItemsOnHooks.Contains(HookComponent)) {
			ItemsOnHooks[HookComponent].Add(ItemToHookUp);
		}
		else {
			TArray<ARRefillObject*> NewActorArray;
			NewActorArray.Add(ItemToHookUp);
			ItemsOnHooks.Add(HookComponent, NewActorArray);
		}

	}
}

void AItemManager::AddItemToHook(UActorComponent * HookComponent, ARRefillObject * ItemToHookUp)
{
	// Casting the hook to a primitive component
	UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(HookComponent);
	if (PrimitiveComponent == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s Couldn't cast to primitive component"), *FString(__FUNCTION__));
	}
	else {
		AddItemToHook(PrimitiveComponent, ItemToHookUp);
	}
}

void AItemManager::AddItemToHook(UActorComponent * HookComponent, AStaticMeshActor * ItemToHookUp)
{
	// Casting the item to a RefillsObject
	ARRefillObject* CastedObject = Cast<ARRefillObject>(ItemToHookUp);
	if (CastedObject == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s Couldn't cast to refill object"), *FString(__FUNCTION__));
	}
	else {
		AddItemToHook(HookComponent, CastedObject);
	}
}

void AItemManager::AddItemToHook(UActorComponent * HookComponent, AActor * ItemToHookUp)
{
	// Casting the item to a RefillsObject
	ARRefillObject* CastedObject = Cast<ARRefillObject>(ItemToHookUp);
	if (CastedObject == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("%s Couldn't cast to refill object"), *FString(__FUNCTION__));
	}
	else {
		// Casting the hook to a primitive component
		UPrimitiveComponent* CastedHook = Cast<UPrimitiveComponent>(HookComponent);

		if (CastedHook != nullptr && SetOfHooks.Contains(CastedHook)) {
			AddItemToHook(HookComponent, CastedObject);
		}
	}
}

void AItemManager::RemoveItemFromHook(UActorComponent * HookComponent, ARRefillObject* Item)
{
	// Remove the item from the map of ItemsOnHooks
	if (HookComponent != nullptr && ItemsOnHooks.Contains(HookComponent)) {
		ItemsOnHooks[HookComponent].Remove(Item);
	}
}

UActorComponent* AItemManager::FindHookOfItem(ARRefillObject * Item)
{
	// Get the Hole component
	UActorComponent* HoleTabComponent = Item->GetComponentByClass(UHoleTabComponent::StaticClass());

	if (HoleTabComponent != nullptr) {

		// Get the constraint
		UActorComponent* HookConstraintAsComponent = Item->GetComponentByClass(UPhysicsConstraintComponent::StaticClass());

		if (HookConstraintAsComponent != nullptr) {
			// Cast it to constraint component
			UPhysicsConstraintComponent* HookConstraint = Cast<UPhysicsConstraintComponent>(HookConstraintAsComponent);

			UActorComponent* HookComponent = nullptr;

			// Get the UActorComponent of the hook by the name of the hook
			if (HookConstraint != nullptr) {
				if (HooknamesToHookComponent.Contains(HookConstraint->ComponentName1.ComponentName)) {
					HookComponent = HooknamesToHookComponent[HookConstraint->ComponentName1.ComponentName];
				}
				else if (HooknamesToHookComponent.Contains(HookConstraint->ComponentName2.ComponentName)) {
					HookComponent = HooknamesToHookComponent[HookConstraint->ComponentName2.ComponentName];
				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("%s: No hook was found"), *FString(__FUNCTION__));
				}
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("%s: No hook constraint was found"), *FString(__FUNCTION__));
			}

			return HookComponent;
		}
	}

	return nullptr;
}

