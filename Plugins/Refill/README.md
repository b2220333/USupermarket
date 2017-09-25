# USupermarket

## Installlation
- Copy all files from plugin's content folder to your content folder
- Add a `Trigger Box` to the world and place it somewhere hidden in the world
- Add an AssetLoader (e.g. CacheAssetLoader) to the world and assign the Trigger Box to it. Also check the `Asset Path`
- Add an ItemManager instance to the world
- Add a RMyCharacter to the world and enable `Auto Posess Player`
- Add to the Character two components `CMovement` and  `CPlaceItem`	 
- Go to the `CPlaceItem` component and assign the `AssetLoader` as well as the `Red Material` which you can find in the Highlight-folder you copied earlier
- To to `Project Settings` -> `Maps & Modes` and choose `RefillGameModeBase` as Default GameMode
- Add a `Post Process Volume`in the world and fit the cube so everything is inside it. Then add the `PP_Outliner` material to `Rendering Features` -> `Post Process Material`
- Add a SavegameManager instance to the world. Assign the AssetLoader and set up the `Savegame file name` and `Save Directory`

## Setting up Shelves:
- Add a BlockingVolume to the scene
- Fit the Brush-Size to the surface
- Do NOT scale the blocking volume
- Add the tag `Refill;Shelf,true;`
- Set Collisions to: 
	- Preset: Custom
	- Collision Enabled: Query only
	- Object Type WorldStatic
	- TraceResponses:
		- Visibility: Ignore
		- Camera: Overlap
		- WorldStatic: Block
		- WorldDynamic: Overlap
		- Pawn: Overlap
		- PhysicsBody: Overlap
		- Vehicle: Overlap
		- Destructible: Overlap
- Enable `Generate Overlap Events`
- The shelf mesh itself should always be WorldStatic and BlockAll

## Adding Hooks
- Place a shelf in the world and attach a StaticMeshComponent with the MetalHold Mesh
- Set Collosion Preset of the MetalHold to `BlockAll`
- Add the Component Tag `Refill;Hook,True;` to the metal hold

## Controls:
- W, S, A, D = Movement
- E = Open inventory
- Left Mouse = Place one item
- "Shift + Left Mouse = Place a row of items"
- "CTRL + Left Mouse = Fill surface with items"
- "MouseWheel = Change Spacing (in combination with Shift or CTRL)"
- "1 = Decrease spacing steps for a smother spacing"
- "2 = Increase spacing step"
- "R = Rotate item"
- "F" = Toggle Placing/Selecting Mode
- "+", "-" = Increase/Decrease stack height
- F6 = Save game
- F9 = Load game

## Assets
- Place your Assets as ``` SM_<AssetName>.uasset``` to the Content/Cache/Items folder
- Additionally add a ``` SM_<AssetName>.png``` in the same folder which then will be displayed in the inventory list
- Additionally add a ``` SM_<AssetName>.json``` file with information about the product. If no json file is present, the name of the item in the inventory list equals the .uasset name.

### JSON file for RefillsItems
The following structure is currently used

```json
{
  "Company": "Balea",  
  "Name": "Tatoo",  
  "Tags": [
    "Refill;Hookable,True;"
  ],  
  "HookPosition": {
    "X": 0.0,
    "Y": 0.0,
    "Z": 8.0
  }
}
```

```HookPosition``` only needs to be set if this item can be attached to a hook.

## Savegame Manager

