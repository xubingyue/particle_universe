/*
-----------------------------------------------------------------------------------------------
This source file is part of the Particle Universe product.

 Copyright (c) 2009 Henry van Merode

Usage of this program is licensed under the terms of the Particle Universe Commercial License.
You can find a copy of the Commercial License in the Particle Universe package.
-----------------------------------------------------------------------------------------------
*/

#include <iostream>
#include "AtlasImageTool.h"
#include "Ogre.h"

int main(int argc, char* argv[])
{
	AtlasImageTool atlasImageTool;
	std::cout << "\n";
	std::cout << "------------------------ Start creating atlas image! ------------------------\n";
	Ogre::String configFileName = Ogre::StringUtil::BLANK;
	if (argv[1])
	{
		configFileName = argv[1];
	}

	try
	{
		atlasImageTool.initialise(configFileName);
		atlasImageTool.process();
		std::cout << "------------------------ End creating atlas image! ------------------------\n";
		std::cout << "\n";
	}

	catch (Ogre::Exception& e)
	{
		std::cout << "\n";
		std::cout << "Processing Error =============> \n" << e.getFullDescription();
		std::cout << "\n";
		return false;
	}

	return 0;
}
//-----------------------------------------------------------------------
void AtlasImageTool::initialise (const Ogre::String& configFileName)
{
	if (!configFileName.empty())
	{
		mConfigFile.load(configFileName);
	}
	else
	{
		mConfigFile.load("atlas.cfg");
	}

	mInputFileNames = Ogre::StringUtil::split(mConfigFile.getSetting("InputImage"), ";, ");
	mInputFrames = Ogre::StringUtil::split(mConfigFile.getSetting("Frame"), ";, ");
	mAlpha = Ogre::StringUtil::split(mConfigFile.getSetting("Alpha"), ";, ");
	mOutputImage = mConfigFile.getSetting("OutputImage");
	mImagePath = mConfigFile.getSetting("ImagePath");
	mAtlasImage.setAlwaysUpdate(false); // Manual compilation to speed things up.
}
//-----------------------------------------------------------------------
void AtlasImageTool::process (void)
{
	Ogre::Root root("", "", "atlas.log");
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(mImagePath, "FileSystem");
	Ogre::StringVector::iterator itInputFileName;
	Ogre::StringVector::iterator itFrame;
	Ogre::StringVector::iterator itAlpha;

	itAlpha = mAlpha.begin();
	if (mInputFrames.empty() || mInputFrames[0] == Ogre::StringUtil::BLANK)
	{
		// No Frames are assigned so just add them
		for (itInputFileName = mInputFileNames.begin(); itInputFileName != mInputFileNames.end(); ++itInputFileName)
		{
			Ogre::String imageFileName = *itInputFileName;
			Ogre::Image image;
			image.load(imageFileName, "General");

			if (itAlpha != mAlpha.end() && *itAlpha != Ogre::StringUtil::BLANK)
			{
				Ogre::Real alpha = Ogre::StringConverter::parseReal(*itAlpha);
				correctAlpha(image, alpha);
				itAlpha++;
			}

			mAtlasImage.addImage(&image);
		}
	}
	else
	{
		// Frames are assigned, so generate intermediate images
		itInputFileName = mInputFileNames.begin();
		Ogre::Real alpha = 1.0f;
		Ogre::String nextImageFileName = *itInputFileName;
		Ogre::Image nextImage;
		itFrame = mInputFrames.begin();
		size_t nextFrame = Ogre::StringConverter::parseUnsignedInt(*itFrame);
		nextImage.load(nextImageFileName, "General");
		size_t frameCounter = 0;

		if (!mAlpha.empty() && mAlpha[0] != Ogre::StringUtil::BLANK)
		{
			itAlpha = mAlpha.begin();
			Ogre::Real alpha = Ogre::StringConverter::parseReal(*itAlpha);
			correctAlpha(nextImage, alpha);
			itAlpha++;
		}

		mAtlasImage.addImage(&nextImage);
		frameCounter++;
		itInputFileName++;
		itFrame++;

		while (itInputFileName != mInputFileNames.end())
		{
			// Get the next filename
			Ogre::Image firstImage(nextImage);
			nextImageFileName = *itInputFileName;
			nextImage.load(nextImageFileName, "General");

			if (itAlpha != mAlpha.end() && *itAlpha != Ogre::StringUtil::BLANK)
			{
				Ogre::Real alpha = Ogre::StringConverter::parseReal(*itAlpha);
				correctAlpha(nextImage, alpha);
				itAlpha++;
			}

			if (itFrame != mInputFrames.end())
			{
				size_t firstFrame = nextFrame;
				nextFrame = Ogre::StringConverter::parseUnsignedInt(*itFrame);
				itFrame++;
				frameCounter++;

				// Generate and add interpolated images to the atlas image
				size_t numberOfFrames = nextFrame - firstFrame;
				for (size_t i = 1; i < numberOfFrames; ++i)
				{
					Ogre::Real fraction = (Ogre::Real)i / (Ogre::Real)numberOfFrames;
					Ogre::Image interpolatedImage;
					size_t pixelSize = Ogre::PixelUtil::getNumElemBytes(firstImage.getFormat());
					size_t bufferSize = firstImage.getWidth() * firstImage.getHeight() * pixelSize;
					Ogre::uchar* data = OGRE_ALLOC_T(Ogre::uchar, bufferSize, Ogre::MEMCATEGORY_GENERAL);
					interpolatedImage.loadDynamicImage(data, firstImage.getWidth(), firstImage.getHeight(), 1, firstImage.getFormat(), true);
					interpolate (interpolatedImage, firstImage, nextImage, fraction);
					mAtlasImage.addImage(&interpolatedImage);
					frameCounter++;
				}
			}
			mAtlasImage.addImage(&nextImage);
			frameCounter++;
			itInputFileName++;
		}
	}

	mAtlasImage._compile();
	mAtlasImage.save(mImagePath + "//" + mOutputImage);
}
//-----------------------------------------------------------------------
void AtlasImageTool::interpolate (Ogre::Image& interpolatedImage, 
								  Ogre::Image& firstImage, 
								  Ogre::Image& nextImage, 
								  Ogre::Real fraction)
{
	size_t numPixels = interpolatedImage.getWidth() * interpolatedImage.getHeight();
	size_t pointer = 0;
	for (size_t i = 0; i < numPixels; ++i)
	{
		Ogre::ColourValue firstColour;
		Ogre::ColourValue nextColour;
		Ogre::ColourValue interpolatedColour;
		Ogre::PixelUtil::unpackColour(&firstColour, firstImage.getFormat(), (firstImage.getData() + pointer));
		Ogre::PixelUtil::unpackColour(&nextColour, nextImage.getFormat(), (nextImage.getData() + pointer));
		interpolatedColour = firstColour + fraction * (nextColour - firstColour);
		Ogre::PixelUtil::packColour(interpolatedColour, interpolatedImage.getFormat(), (interpolatedImage.getData() + pointer));
		pointer += Ogre::PixelUtil::getNumElemBytes(interpolatedImage.getFormat());
	}
}
//-----------------------------------------------------------------------
void AtlasImageTool::correctAlpha (Ogre::Image& image, Ogre::Real alphaCorrection)
{
	size_t numPixels = image.getWidth() * image.getHeight();
	size_t pointer = 0;
	for (size_t i = 0; i < numPixels; ++i)
	{
		Ogre::ColourValue colour;
		Ogre::PixelUtil::unpackColour(&colour, image.getFormat(), (image.getData() + pointer));
		colour.a *= alphaCorrection;
		Ogre::PixelUtil::packColour(colour, image.getFormat(), (image.getData() + pointer));
		pointer += Ogre::PixelUtil::getNumElemBytes(image.getFormat());
	}
}
