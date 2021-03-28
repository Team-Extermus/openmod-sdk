#include "cbase.h"
#include "gamemounter.h"
#include "filesystem.h"
#include "steam/steam_api.h"

// brute forces our search paths, reads the users steam configs
// to determine any additional steam library directories people have
// as there's no other way to currently mount a different game (css) 
// if it's located in a different library without an absolute path
void MountPath( KeyValues* pGame )
{
	const char* szGameName = pGame->GetName();
	const bool bRequired = pGame->GetBool( "required", false );

	if ( !steamapicontext || !steamapicontext->SteamApps() )
	{
		if ( bRequired )
			Error( "Failed to mount required game: %s, unable to determine app install path.\nPlease make sure Steam is running, and the game is installed properly.\n", szGameName );
		else
			Msg( "Skipping %s, unable to get app install path.\n", szGameName );

		return;
	}

	char szPath[ MAX_PATH * 2 ];
	int ccFolder = steamapicontext->SteamApps()->GetAppInstallDir( pGame->GetUint64( "appid" ), szPath, sizeof( szPath ) );

	if ( ccFolder > 0 )
	{
		ConColorMsg( Color( 90, 240, 90, 255 ), "Mounting %s\n", szGameName );

		KeyValues *pPaths = pGame->FindKey( "paths" );

		if ( !pPaths )
			return;

		for ( KeyValues *pPath = pPaths->GetFirstSubKey(); pPath; pPath = pPath->GetNextKey() )
		{
			if ( !FStrEq( pPath->GetName(), "local" ) )
				continue;

			char szTempPath[ MAX_PATH * 2 ];
			Q_strncpy( szTempPath, szPath, ARRAYSIZE( szTempPath ) );

			V_AppendSlash( szTempPath, ARRAYSIZE( szTempPath ) );
			V_strncat( szTempPath, pPath->GetString(), ARRAYSIZE( szTempPath ) );

			g_pFullFileSystem->AddSearchPath( szTempPath, "GAME" );
			ConColorMsg( Color( 144, 238, 144, 255 ), "\tAdding path: %s\n", pPath->GetString() );
		}
	}
	else if ( bRequired )
		Error( "Failed to mount required game: %s\n", szGameName );
	else
		Warning( "%s not found on system. Skipping.\n", szGameName );
}

void AddRequiredSearchPaths()
{
	KeyValues *pMountFile = new KeyValues( "gamemounting.txt" );
	pMountFile->LoadFromFile( g_pFullFileSystem, "gamemounting.txt", "MOD" );

	for( KeyValues *pGame = pMountFile->GetFirstTrueSubKey(); pGame; pGame = pGame->GetNextTrueSubKey() )
		MountPath( pGame );

	pMountFile->deleteThis();
}