/*
-----------------------------------------------------------------------------------------------
This source file is part of the Particle Universe product.

 Copyright (c) 2009 Henry van Merode

Usage of this program is licensed under the terms of the Particle Universe Commercial License.
You can find a copy of the Commercial License in the Particle Universe package.
-----------------------------------------------------------------------------------------------
*/

#ifndef __ATLAS_IMAGE_TOOL_H__
#define __ATLAS_IMAGE_TOOL_H__

#include "ParticleUniverseAtlasImage.h"
#include "OgreConfigFile.h"
#include "OgreString.h"

class AtlasImageTool
{
	public:

		AtlasImageTool(void) : 
			mAtlasImage(), 
			mConfigFile(),
			mInputFileNames(),
			mInputFrames(),
			mAlpha(),
			mOutputImage("atlas.jpg"),
			mImagePath(Ogre::StringUtil::BLANK) {};
		virtual ~AtlasImageTool(void){};

		/** Read the configuration file.
		*/
		void initialise (const Ogre::String& configFileName);

		/** Create the atlas image.
		*/
		void process (void);

		/** Create an interpolated image.
		*/
		void interpolate (Ogre::Image& interpolatedImage, 
			Ogre::Image& firstImage, 
			Ogre::Image& nextImage,
			Ogre::Real fraction);

		/** Correct the alpha component.
		*/
		void correctAlpha (Ogre::Image& image, Ogre::Real alphaCorrection);

	protected:
		ParticleUniverse::AtlasImage mAtlasImage;
		Ogre::ConfigFile mConfigFile;
		Ogre::StringVector mInputFileNames;
		Ogre::StringVector mInputFrames;
		Ogre::StringVector mAlpha;
		Ogre::String mOutputImage;
		Ogre::String mImagePath;

};

#endif
