#ifndef _IgnorePlistFix
#include <Geode/Geode.hpp>
#include <Geode/modify/CCSpriteFrameCache.hpp>

using namespace geode::prelude;
// joins the dir with the file like a good file it is
std::string joinPath(std::string dir, std::string file) {
	std::replace(dir.begin(), dir.end(), '\\', '/');
	std::replace(file.begin(), file.end(), '\\', '/');

	while (!dir.empty() && dir.back() == '/')
		dir.pop_back();
	while (!file.empty() && file.front() == '/')
		file.erase(0, 1);

	if (dir.empty())
		return file;
	if (file.empty())
		return dir;
	return dir + "/" + file;
}

/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2008-2010 Ricardo Quesada
Copyright (c) 2009      Jason Booth
Copyright (c) 2009      Robert J Payne
Copyright (c) 2011      Zynga Inc.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
// from https://github.com/altalk23/cocos2d-x-gd/blob/6bccfe7aecdbc32977395d50abcf385627b8f688/cocos2dx/sprite_nodes/CCSpriteFrameCache.cpp#L79
inline void addSpriteFramesWithDictionaryOnlyLoadingTextureIfNeeded(CCDictionary *dictionary, const std::string &texturePath) {
	if (!dictionary)
		return;

	auto cache = CCSpriteFrameCache::get();
	auto metadata = typeinfo_cast<CCDictionary *>(dictionary->objectForKey("metadata"));
	auto frames = typeinfo_cast<CCDictionary *>(dictionary->objectForKey("frames"));
	if (!frames)
		return;

	int format = metadata ? metadata->valueForKey("format")->intValue() : 0;
	CCAssert(format >= 0 && format <= 3, "Unsupported plist format");

	CCTexture2D *texture = nullptr;
	for (auto const& [frameName, frameDict] : CCDictionaryExt<std::string,CCDictionary*>(frames)) {
		if (cache->m_pSpriteFrames->objectForKey(frameName))
			continue;
		// only tries loading if one, it should be here, and 2 if it hasn't yet
		if (!texture) {
			texture = CCTextureCache::sharedTextureCache()->addImage(texturePath.c_str(), false);
			if (!texture) return;
		}

		CCSpriteFrame *frame = new CCSpriteFrame(); 
		// formats
		switch (format) {
		case 0: {
			float x = frameDict->valueForKey("x")->floatValue();
			float y = frameDict->valueForKey("y")->floatValue();
			float w = frameDict->valueForKey("width")->floatValue();
			float h = frameDict->valueForKey("height")->floatValue();
			float ox = frameDict->valueForKey("offsetX")->floatValue();
			float oy = frameDict->valueForKey("offsetY")->floatValue();
			float ow = std::abs(frameDict->valueForKey("originalWidth")->intValue());
			float oh = std::abs(frameDict->valueForKey("originalHeight")->intValue());

			frame->initWithTexture(
			    texture,
			    CCRectMake(x, y, w, h),
			    false,
			    CCPointMake(ox, oy),
			    CCSizeMake(ow, oh));
			break;
		}

		case 1:
		case 2: {
			CCRect rect =
			    CCRectFromString(frameDict->valueForKey("frame")->getCString());

			bool rotated =
			    (format == 2) && frameDict->valueForKey("rotated")->boolValue();

			CCPoint offset =
			    CCPointFromString(frameDict->valueForKey("offset")->getCString());

			CCSize sourceSize =
			    CCSizeFromString(frameDict->valueForKey("sourceSize")->getCString());

			frame->initWithTexture(texture, rect, rotated, offset, sourceSize);
			break;
		}

		case 3: {
			CCSize spriteSize =
			    CCSizeFromString(frameDict->valueForKey("spriteSize")->getCString());

			CCPoint spriteOffset =
			    CCPointFromString(frameDict->valueForKey("spriteOffset")->getCString());

			CCSize spriteSourceSize =
			    CCSizeFromString(frameDict->valueForKey("spriteSourceSize")->getCString());

			CCRect textureRect =
			    CCRectFromString(frameDict->valueForKey("textureRect")->getCString());

			bool rotated =
			    frameDict->valueForKey("textureRotated")->boolValue();

			if (auto aliases =
			        static_cast<CCArray *>(frameDict->objectForKey("aliases"))) {
				auto key = new CCString(frameName);
				for (unsigned i = 0; i < aliases->count(); ++i) {
					auto alias =
					    static_cast<CCString *>(aliases->objectAtIndex(i));
					cache->m_pSpriteFramesAliases->setObject(
					    key,
					    alias->getCString());
				}
				key->release();
			}

			frame->initWithTexture(
			    texture,
			    CCRectMake(textureRect.origin.x,
			               textureRect.origin.y,
			               spriteSize.width,
			               spriteSize.height),
			    rotated,
			    spriteOffset,
			    spriteSourceSize);
			break;
		}
		}

		cache->m_pSpriteFrames->setObject(frame, frameName);
		frame->release();
	}
}

std::string getPlistForQuality(const char *plist, cocos2d::TextureQuality quality) {
	std::string result(plist);

	size_t dot = result.find_last_of('.');
	if (dot == std::string::npos)
		dot = result.size();

	std::string base = result.substr(0, dot);

	bool hasHD =
	    base.size() >= 3 &&
	    base.compare(base.size() - 3, 3, "-hd") == 0;

	bool hasUHD =
	    base.size() >= 4 &&
	    base.compare(base.size() - 4, 4, "-uhd") == 0;

	if (!hasHD && !hasUHD) {
		switch (quality) {
		case cocos2d::TextureQuality::kTextureQualityHigh:
			result.insert(dot, "-uhd");
			break;

		case cocos2d::TextureQuality::kTextureQualityMedium:
			result.insert(dot, "-hd");
			break;

		default:
			break;
		}
	}

	return result;
}

class $modify(RewrittenSpriteFrames, CCSpriteFrameCache) {
	static void onModify(auto &self) {
		// "If you’re reimplementing the original function and do not call the original, use Priority::Last with setHookPriorityPre."
		// also makes sure it hooks last
		if (!self.setHookPriorityPre("CCSpriteFrameCache::addSpriteFramesWithFile", Priority::Last)) {
            log::warn("Failed to set CCSpriteFrameCache::addSpriteFramesWithFile hook priority, texture loader might break!");
        }
	}

	void addSpriteFramesWithFile(const char *plist) {
		auto fileUtils = CCFileUtils::sharedFileUtils();
		if (fileUtils->isAbsolutePath(plist)) { // Absolute path, Do not mess with!
			return CCSpriteFrameCache::addSpriteFramesWithFile(plist);
		}
		std::string plistStr = getPlistForQuality(plist, CCDirector::get()->getLoadedTextureQuality());

		// already preloaded
		if (!m_pLoadedFileNames->insert(plistStr).second)
			return;

		for (const auto &dir : fileUtils->getSearchPaths()) {
			std::string fullPath = joinPath(dir, plistStr);

			if (!fileUtils->isFileExist(fullPath)) // does the search path find it
				continue;

			auto dict = CCDictionary::createWithContentsOfFileThreadSafe(fullPath.c_str()); // robtop method in creating plist arrays

			if (!dict) // failed?
				continue;

			std::string texturePath;
			// robtop checks if textureFileName exists... I join the dir together to make sure this search path is used
			if (auto metadata = typeinfo_cast<CCDictionary *>(dict->objectForKey("metadata"))) {
				texturePath = joinPath(dir, metadata->valueForKey("textureFileName")->getCString());
			}
			// Doesn't contain metadata for textureFileName
			if (texturePath.empty()) {
				texturePath = plistStr;
				size_t pos = texturePath.find_last_of('.');
				if (pos != std::string::npos)
					texturePath.erase(pos);
				texturePath += ".png";
			}
			// inlined version of addSpriteFramesWithDictionary as it private also makes it so it only loads the image if needed
			addSpriteFramesWithDictionaryOnlyLoadingTextureIfNeeded(dict, texturePath);
			dict->release();
		}
	}
};
#endif