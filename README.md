# USupermarket

## Controls:
- Left Mouse = Place one item
- "Shift + Left Mouse = Place a row of items"
- "CTRL + Left Mouse = Fill surface with items"
- "MouseWheel = Change Spacing (in combination with Shift or CTRL)"
- "1 = Decrease spacing steps for a smother spacing"
- "2 = Increase spacing step"
- "R = Rotate item"
- "F" = Toggle Placing/Selecting Mode
- "+", "-" = Increase/Decrease stack height
- "TAB" = Open list of spawnable items. Click on an item to spawn it. TAB again to close the list

## Setting up Shelves:
- Add a BlockingVolume to the scene
- Fit the Brush-Size to the surface
- Do NOT scale the blocking volume
- Set Collisions to: 
	- Preset: Custom
	- Collision Enabled: Query only
	- Object Type WorldStatic
	- TraceResponses:
		- Visibility: Ignore
		- Camera: Block
		- WorldStatic: Block
		- WorldDynamic: Overlap
		- Pawn: Block
		- PhysicsBody: Overlap
		- Vehicle: Block
		- Destructible: Block
- Check 'Generate Overlap Events'

