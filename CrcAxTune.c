/*
 *						(c) Gealtec, de German Ortiz
 *						    All rights reserved.
 *							Todos los derechos reservados.
 *
 *	Title:				CrcAxTune.c
 *
 *	Description:		
 *
 *	Author:				German
 *						German@gealtec.com
 *
 *	Revision History:
 *
 *	Rev #	Date		Who				Comments
 *	------	----------	-------------	------------------------------------
 *	1.0		10/01/2013		German			Initial file released
 *
 */

/*
 * Includes
 */

#include	<stdio.h>
#include	<string.h>

#include	"aes256.h"
#include	"md5.h"

/*
 * Defines
 */

#define	NAME_LEN_MAX		1024
#define	READ_BUFFER_LEN		1024

#define HASHLEN				16
typedef char 				HASH[HASHLEN];

/*
 * Local Vars.
 */

char			Url				[NAME_LEN_MAX];
char			Exe				[NAME_LEN_MAX];
char			Ver				[NAME_LEN_MAX];
char			Ins				[NAME_LEN_MAX];
char			Hfs				[NAME_LEN_MAX];

FILE	*		FExe;
FILE	*		FIns;

MD5_CTX			Md5Ctx;
HASH			HA1;

unsigned char	ReadBuffer		[READ_BUFFER_LEN];
size_t			ReadBufferLen;
size_t			ExeFileLen;

/*
 * The Code
 */

void
MostrarModoDeUso ( char *NombreAplicacion )
{
	printf	( "\r\nUsar:  %s <NombreEjecutableInstalador.exe> <UrlDestino> <Version> <NombreArchivoConInstruccionesDeDescarga.dins>\r\n\r\n" , NombreAplicacion );
}

void
HashToAscii	( HASH HA1 , char * Hsh	)
{
	int		i;
	char	TempStr [16];

	memset	( Hsh 		, 0 , HASHLEN * 2 + 1 );
	memset	( TempStr	, 0 , 16 );
	for ( i = 0 ; i < HASHLEN ; i++ )
	{
		sprintf	( TempStr , "%2.2X" , (unsigned char)(HA1[i]&255) );
		strncat	( Hsh , TempStr , 2 );
	}
}

int
main(int argc, char* argv[])
{
	/*	Tendré los argumentos correctos ?			*/
	if ( argc != 5 )
	{
		MostrarModoDeUso ( argv[0] );
		return 1;
	}

	/*	Copiar los nombres de todos los argumentos	*/
	strncpy	( Exe , argv[1] , NAME_LEN_MAX );
	strncpy	( Url , argv[2] , NAME_LEN_MAX );
	strncpy	( Ver , argv[3] , NAME_LEN_MAX );
	strncpy	( Ins , argv[4] , NAME_LEN_MAX );

	/*	Abrir el Exe ...							*/
	if ( ( FExe = fopen ( Exe , "rb" )) == NULL )
	{
		printf ( "%s: Error abriendo archivo %s\r\n" , argv[0] , Exe );
		return 1;
	}

	/*	Crear el archivo de Salida					*/
	if ( ( FIns = fopen ( Ins , "wb" )) == NULL )
	{
		printf ( "%s: Error creando archivo %s\r\n" , argv[0] , Ins );
		fclose	( FExe );
		return 1;
	}

	/*	Inicializar la funcion de hashing			*/
	MD5Init		( &Md5Ctx );

	/*	Leer por completo el archivo de entrada y calcular el hash	*/
	ExeFileLen	= 0;

	while ( ( ReadBufferLen = fread ( ReadBuffer , 1 , READ_BUFFER_LEN , FExe )) != 0 )
	{
		ExeFileLen	+=	ReadBufferLen;
		MD5Update	( &Md5Ctx , ReadBuffer , ReadBufferLen );
	}

	/*	Agregar al Hash la version y el link de descarga	*/
	MD5Update	( &Md5Ctx , Ver , strlen ( Ver ) );
	MD5Update	( &Md5Ctx , Url , strlen ( Url ) );
	MD5Update	( &Md5Ctx , "gealtec" , strlen ( "gealtec" ) );
	MD5Update	( &Md5Ctx , "." , strlen ( "." ) );

	/*	Cerrar archivo y Hash						*/
	fclose		( FExe );
	MD5Final	( HA1 ,	&Md5Ctx );
	HashToAscii	( HA1 , Hfs		);

	/*	Crear el archivo de instruccion				*/
	fputs		( "AXTUNE_UPGRADE_INFO"		, FIns );
	fputs		( "\r\n"		, FIns );
	fputs		( Ver		, FIns );
	fputs		( "\r\n"		, FIns );
	fputs		( Url 		, FIns );
	fputs		( "\r\n"		, FIns );
	fprintf		( FIns		, "%8.8lX\r\n" , (unsigned long)ExeFileLen );
	fputs		( Hfs 		, FIns );
	fputs		( "\r\n"		, FIns );

	/*	Cerrar archivo de Instruccion y listo		*/
	fclose 		( FIns );

	/*	Mostrar el resultado en pantalla			*/
	puts		( "AXTUNE_UPGRADE_INFO" );
	puts		( Ver );
	puts		( Url );
	printf		( "%8.8lX\r\n" , (unsigned long)ExeFileLen );
	puts		( Hfs );
	return 		0;
}

