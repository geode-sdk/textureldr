# Texture Loader

Everyone's favorite Texture Pack manager for Geometry Dash!

## For Pack Creators

We provide a few features to make your pack look nicer on our UI. 
* You can add a custom pack icon by having `pack.png` on the root of the pack
* You can give it a name and other info by having `pack.json` on the root of the pack

You can distribute your pack as a zip, but also work on it as a local folder in the `packs/` directory.

For reference, see this example pack we provided: https://github.com/geode-sdk/textureldr/releases/download/v1.5.0/fancy.zip

The zip for that pack looks like this: \
![image](https://github.com/geode-sdk/textureldr/assets/26722564/f94fbca0-1802-4fce-8601-4dd0119c9a18)

> Make sure to put the files are the root of the zip! otherwise Texture Loader might not find them properly.

This pack will:
* Change the background gradient image `GJ_gradientBG-hd.png`
* Edit the textures for a mod (geode.loader) [More info about that](#changing-mod-resources)
* Have a custom icon `pack.png`

The structure of pack.json is as follows:
```json
{
	"textureldr": "1.5.0",
	"name": "Name of the pack",
	"id": "author.some-id",
	"version": "1.0.0",
	"author": "your name"
}
```
The first value, `textureldr` must be the version of Texture Loader you're targetting. Currently, only the `name` value is used for anything

## Changing Mod Resources
To edit the resources for a mod, simply put them in a folder that has the same name as the mod id.

For example: *Currently* BetterInfo has this texture: \
![image](https://github.com/geode-sdk/textureldr/assets/26722564/6d887c5c-854d-4278-a5b0-08a823a52d37)

You can easily change it in your texture pack by doing this: \
![image](https://github.com/geode-sdk/textureldr/assets/26722564/776b9e06-555a-49e9-8b99-10d93338831b) \
![image](https://github.com/geode-sdk/textureldr/assets/26722564/fa58a2bf-1406-49a5-9e5e-9c1c111e324d) \

