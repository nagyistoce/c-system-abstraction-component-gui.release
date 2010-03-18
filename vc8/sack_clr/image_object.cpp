#include <imglib/imagestruct.h>

using namespace System;
///
///  what?!
///
namespace sack {
	public ref class Image {
		//System::
		IntPtr image;
	public: 
		Image( CTEXTSTR file )
		{
			image = (IntPtr)LoadImageFile( file );
		}
		Image( int width, int height )
		{
			image = (IntPtr)MakeImageFile( width, height );
		}

	};
};