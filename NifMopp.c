#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

typedef int (__stdcall * fnGenerateMoppCode)( int nVerts, const float * verts, int nTris, const unsigned short * tris );
typedef int (__stdcall * fnGenerateMoppCodeWithSubshapes)( int nShapes, const int * shapes,
															int nVerts, const float * verts,
															int nTris, const unsigned short * tris );
typedef int (__stdcall * fnRetrieveMoppCode)( int nBuffer, char * buffer );
typedef int (__stdcall * fnRetrieveMoppScale)( float * value );
typedef int (__stdcall * fnRetrieveMoppOrigin)( float * value );

int main( int argc, char ** argv )
{
	if ( argc != 2 )
		return 1;

	// load mesh data
	FILE * f = fopen( argv[1], "rb" );
	int * shapes = NULL;
	float * verts = NULL;
	unsigned short * tris = NULL;
	int nShapes = 0;
	int nVerts = 0;
	int nTris = 0;
	bool isValid = false;
	do {
		if ( !f )
			break;
		unsigned int tmp;
		if ( fread( &tmp, sizeof( unsigned int ), 1, f ) != 1 )
			break;
		if ( tmp != 0x4853454D )	// "MESH"
			break;
		// read sub-shape count and data
		if ( fread( &tmp, sizeof( unsigned int ), 1, f ) != 1 )
			break;
		if ( tmp > 65536u )
			break;
		if ( tmp ) {
			nShapes = (int) tmp;
			shapes = (int *) malloc( tmp * sizeof( int ) );
			if ( !shapes )
				break;
			if ( fread( shapes, sizeof( int ), tmp, f ) != tmp )
				break;
		}
		// read vertex count and data
		if ( fread( &tmp, sizeof( unsigned int ), 1, f ) != 1 )
			break;
		if ( !tmp || tmp > 65536u )
			break;
		nVerts = (int) tmp;
		tmp = tmp * 3u;
		verts = (float *) malloc( tmp * sizeof( float ) );
		if ( !verts )
			break;
		if ( fread( verts, sizeof( float ), tmp, f ) != tmp )
			break;
		// read triangle count and data
		if ( fread( &tmp, sizeof( unsigned int ), 1, f ) != 1 )
			break;
		if ( !tmp || tmp > 131072u )
			break;
		nTris = (int) tmp;
		tmp = tmp * 3u;
		tris = (unsigned short *) malloc( tmp * sizeof( unsigned short ) );
		if ( !tris )
			break;
		if ( fread( tris, sizeof( unsigned short ), tmp, f ) != tmp )
			break;
		isValid = true;
	} while ( false );
	fclose( f );
	if ( !isValid ) {
		free( shapes );
		free( verts );
		free( tris );
		return 1;
	}

	// load NifMopp library and generate MOPP code
	int moppCodeLen = 0;
	char * moppCodeData = NULL;
	float moppOrigin[3] = { 0.0f, 0.0f, 0.0f };
	float moppScale = 1.0f;
	{
		HMODULE hMoppLib = LoadLibraryA( "NifMopp.dll" );
		if ( hMoppLib ) {
			fnGenerateMoppCode GenerateMoppCode = (fnGenerateMoppCode) GetProcAddress( hMoppLib, "GenerateMoppCode" );
			fnRetrieveMoppCode RetrieveMoppCode = (fnRetrieveMoppCode) GetProcAddress( hMoppLib, "RetrieveMoppCode" );
			fnRetrieveMoppScale RetrieveMoppScale =
				(fnRetrieveMoppScale) GetProcAddress( hMoppLib, "RetrieveMoppScale" );
			fnRetrieveMoppOrigin RetrieveMoppOrigin =
				(fnRetrieveMoppOrigin) GetProcAddress( hMoppLib, "RetrieveMoppOrigin" );
			fnGenerateMoppCodeWithSubshapes GenerateMoppCodeWithSubshapes =
				(fnGenerateMoppCodeWithSubshapes) GetProcAddress( hMoppLib, "GenerateMoppCodeWithSubshapes" );
			if ( RetrieveMoppCode && RetrieveMoppScale && RetrieveMoppOrigin ) {
				if ( nShapes > 0 && GenerateMoppCodeWithSubshapes )
					moppCodeLen = GenerateMoppCodeWithSubshapes( nShapes, shapes, nVerts, verts, nTris, tris );
				else if ( GenerateMoppCode )
					moppCodeLen = GenerateMoppCode( nVerts, verts, nTris, tris );
			}
			if ( moppCodeLen > 0 ) {
				moppCodeData = (char *) malloc( (size_t) moppCodeLen );
				if ( !moppCodeData )
					moppCodeLen = 0;
				else
					RetrieveMoppCode( moppCodeLen, moppCodeData );
			}
			if ( moppCodeLen > 0 ) {
				RetrieveMoppOrigin( moppOrigin );
				RetrieveMoppScale( &moppScale );
			}
			FreeLibrary( hMoppLib );
		}
	}
	free( shapes );
	free( verts );
	free( tris );
	if ( moppCodeLen < 1 )
		return 1;

	// save MOPP code, overwriting the input file
	f = fopen( argv[1], "wb" );
	if ( f ) {
		unsigned int tmp = 0x50504F4D;	// "MOPP"
		fwrite( &tmp, sizeof( unsigned int ), 1, f );
		fwrite( moppOrigin, sizeof( float ), 3, f );
		fwrite( &moppScale, sizeof( float ), 1, f );
		fwrite( &moppCodeLen, sizeof( int ), 1, f );
		fwrite( moppCodeData, sizeof( char ), (size_t) moppCodeLen, f );
		fclose( f );
	}
	free( moppCodeData );

	return 0;
}
