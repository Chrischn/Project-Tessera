Scripts in Godot can be attached to "Scenes" in order for them to be called once a scene is loaded, which is why I chose to name them after the scenes they are attached to.

Scripts can also be attached to "Nodes", like in case of "PlayerPivot.gd" that controls the camera inside the 3D gameworld.

Even though I haven't tried it yet, I'm fairly certain that scripts can be called from other scripts aswell.

In case of "Global.gd" the script is called upon launching the game and is therefor autoloaded by the project itself (Project>Project Settings>Globals>Autoload)