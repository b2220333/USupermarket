// Provides information of the product

#pragma once

struct FRefillObjectInfo
{
public:

	// Company name
	FString Company;

	// Product name
	FString Name;

	// Actor tags
	TArray<FName> Tags;

	// Can this item be placed on a hook?
	bool bCanBeHookedUp;

	// The position of the hole from where this item gets attached to the hook
	FVector HolePosition;

	// Returns the company and product name separated by a colon
	FString GetDisplayName() {
		if (Company.IsEmpty()) return Name;
		return Company + FString(": ") + Name;
	}
};