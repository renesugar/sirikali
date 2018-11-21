/*
 *
 *  Copyright (c) 2018
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "engines.h"

#include "engines/ecryptfs.h"
#include "engines/cryfs.h"
#include "engines/gocryptfs.h"
#include "engines/encfs.h"
#include "engines/sshfs.h"
#include "engines/unknown.h"
#include "engines/securefs.h"

#include "utility.h"

engines::engine::~engine()
{
}

engines::engine::engine( engines::engine::BaseOptions o ) :
	m_Options( std::move( o ) )
{
}

QString engines::engine::executableFullPath() const
{
	return utility::executableFullPath( this->name() ) ;
}

bool engines::engine::isInstalled() const
{
	return !this->isNotInstalled() ;
}

bool engines::engine::isNotInstalled() const
{
	return this->executableFullPath().isEmpty() ;
}

bool engines::engine::unknown() const
{
	return this->name().isEmpty() ;
}

bool engines::engine::known() const
{
	return !this->unknown() ;
}

bool engines::engine::setsCipherPath() const
{
	return m_Options.setsCipherPath ;
}

bool engines::engine::autoMountsOnCreate() const
{
	return m_Options.autoMountsOnCreate ;
}

bool engines::engine::hasGUICreateOptions() const
{
	return m_Options.hasGUICreateOptions ;
}

const QStringList& engines::engine::names() const
{
	return m_Options.names ;
}

const QStringList& engines::engine::fuseNames() const
{
	return m_Options.fuseNames ;
}

const QStringList& engines::engine::configFileNames() const
{
	return m_Options.configFileNames ;
}

const QString& engines::engine::name() const
{
	if( m_Options.names.isEmpty() ){

		static QString s ;
		return  s ;
	}else{
		return m_Options.names.first() ;
	}
}

const QString& engines::engine::configFileName() const
{
	if( m_Options.configFileNames.isEmpty() ){

		static QString s ;
		return  s ;
	}else{
		return m_Options.configFileNames.first() ;
	}
}

QString engines::engine::setConfigFilePath( const QString& e ) const
{
	if( m_Options.configFileArgument.isEmpty() ){

		return QString() ;
	}else{
		return m_Options.configFileArgument + " " + e ;
	}
}

engines::engine::status engines::engine::notFoundCode() const
{
	return m_Options.notFoundCode ;
}

const engines& engines::instance()
{
	static engines v ;
	return v ;
}

QStringList engines::supported()
{
	if( utility::platformIsWindows() ){

		return { "Securefs","Encfs","Sshfs" } ;

	}else if( utility::platformIsOSX() ){

		return { "Cryfs","Gocryptfs","Securefs","Encfs" } ;
	}else{
		return { "Cryfs","Gocryptfs","Securefs","Encfs","Ecryptfs","Sshfs" } ;
	}
}

engines::engines()
{
	m_backends.emplace_back( std::make_unique< unknown >() ) ;
	m_backends.emplace_back( std::make_unique< securefs >() ) ;
	m_backends.emplace_back( std::make_unique< gocryptfs >() ) ;
	m_backends.emplace_back( std::make_unique< cryfs >() ) ;
	m_backends.emplace_back( std::make_unique< encfs >() ) ;
	m_backends.emplace_back( std::make_unique< ecryptfs >() ) ;
	m_backends.emplace_back( std::make_unique< sshfs >() ) ;
}

const engines::engine& engines::getByName( const engines::engine::options& e ) const
{
	return this->getByName( e.type ) ;
}

template< typename T,typename Function >
const engines::engine& _get_engine( const T& engines,const QString& e,Function function )
{
	const auto data = engines.data() ;

	for( size_t i = 1 ; i < engines.size() ; i++ ){

		const auto& s = *( data + i ) ;

		const auto& m = function( *s ) ;

		for( int z = 0 ; z < m.size() ; z++ ){

			if( e == m.at( z ) ){

				return *s ;
			}
		}
	}

	return **data ;
}

const engines::engine& engines::getByFuseName( const QString& e ) const
{
	return _get_engine( m_backends,e,[]( const engines::engine& s ){ return s.fuseNames() ; } ) ;
}

const engines::engine& engines::getByName( const QString& e ) const
{
	return _get_engine( m_backends,e.toLower(),[]( const engines::engine& s ){ return s.names() ; } ) ;
}

engines::engine::cmdStatus::cmdStatus()
{
}

engines::engine::cmdStatus::cmdStatus(engines::engine::status s,int c,const QString& e ) :
	m_exitCode( c ),m_status( s )
{
	this->message( e ) ;
}

engines::engine::cmdStatus::cmdStatus( engines::engine::status s,const QString& e ) :
	m_status( s )
{
	this->message( e ) ;
}

engines::engine::cmdStatus::cmdStatus( int s,const QString& e ) :
	m_exitCode( s )
{
	this->message( e ) ;
}

engines::engine::status engines::engine::cmdStatus::status() const
{
	return m_status ;
}

bool engines::engine::cmdStatus::operator==( engines::engine::status s ) const
{
	return m_status == s ;
}

bool engines::engine::cmdStatus::operator!=( engines::engine::status s ) const
{
	return m_status != s ;
}

QString engines::engine::cmdStatus::toMiniString() const
{
	return m_message ;
}

QString engines::engine::cmdStatus::toString() const
{
	switch( m_status ){

	case engines::engine::status::success :

		/*
		 * Should not get here
		 */

		return "Success" ;

	case engines::engine::status::volumeCreatedSuccessfully :

		return QObject::tr( "Volume Created Successfully." ) ;

	case engines::engine::status::cryfsBadPassword :

		return QObject::tr( "Failed To Unlock A Cryfs Volume.\nWrong Password Entered." ) ;

	case engines::engine::status::sshfsBadPassword :

		return QObject::tr( "Failed To Connect To The Remote Computer.\nWrong Password Entered." ) ;

	case engines::engine::status::encfsBadPassword :

		return QObject::tr( "Failed To Unlock An Encfs Volume.\nWrong Password Entered." ) ;

	case engines::engine::status::gocryptfsBadPassword :

		return QObject::tr( "Failed To Unlock A Gocryptfs Volume.\nWrong Password Entered." ) ;

	case engines::engine::status::ecryptfsBadPassword :

		return QObject::tr( "Failed To Unlock An Ecryptfs Volume.\nWrong Password Entered." ) ;

	case engines::engine::engine::status::ecryptfsIllegalPath :

		return QObject::tr( "A Space Character Is Not Allowed In Paths When Using Ecryptfs Backend And Polkit." ) ;

	case engines::engine::status::ecrypfsBadExePermissions :

		return QObject::tr( "This Backend Requires Root's Privileges And An attempt To Acquire Them Has Failed." ) ;

	case engines::engine::status::securefsBadPassword :

		return QObject::tr( "Failed To Unlock A Securefs Volume.\nWrong Password Entered." ) ;

	case engines::engine::status::sshfsNotFound :

		return QObject::tr( "Failed To Complete The Request.\nSshfs Executable Could Not Be Found." ) ;

	case engines::engine::status::backEndDoesNotSupportCustomConfigPath :

		return QObject::tr( "Backend Does Not Support Custom Configuration File Path." ) ;

	case engines::engine::status::cryfsNotFound :

		return QObject::tr( "Failed To Complete The Request.\nCryfs Executable Could Not Be Found." ) ;

	case engines::engine::status::cryfsMigrateFileSystem :

		return QObject::tr( "This Volume Of Cryfs Needs To Be Upgraded To Work With The Version Of Cryfs You Are Using.\n\nThe Upgrade is IRREVERSIBLE And The Volume Will No Longer Work With Older Versions of Cryfs.\n\nTo Do The Upgrade, Check The \"Upgrade File System\" Option And Unlock The Volume Again." ) ;

	case engines::engine::status::encfsNotFound :

		return QObject::tr( "Failed To Complete The Request.\nEncfs Executable Could Not Be Found." ) ;

	case engines::engine::status::ecryptfs_simpleNotFound :

		return QObject::tr( "Failed To Complete The Request.\nEcryptfs-simple Executable Could Not Be Found." ) ;

	case engines::engine::status::gocryptfsNotFound :

		return QObject::tr( "Failed To Complete The Request.\nGocryptfs Executable Could Not Be Found." ) ;

	case engines::engine::status::securefsNotFound :

		return QObject::tr( "Failed To Complete The Request.\nSecurefs Executable Could Not Be Found." ) ;

	case engines::engine::status::failedToCreateMountPoint :

		return QObject::tr( "Failed To Create Mount Point." ) ;

	case engines::engine::status::failedToLoadWinfsp :

		return QObject::tr( "Backend Could Not Load WinFsp. Please Make Sure You Have WinFsp Properly Installed" ) ;

	case engines::engine::status::unknown :

		return QObject::tr( "Failed To Unlock The Volume.\nNot Supported Volume Encountered." ) ;

	case engines::engine::status::backendFail :

		;
	}

	auto e = QObject::tr( "Failed To Complete The Task And Below Log was Generated By The Backend.\n" ) ;
	return e + "\n----------------------------------------\n" + m_message ;
}

void engines::engine::cmdStatus::message( const QString& e )
{
	m_message = e ;

	while( true ){

		if( m_message.endsWith( '\n' ) ){

			m_message.truncate( m_message.size() - 1 ) ;
		}else{
			break ;
		}
	}
}

engines::engine::Options::Options( QStringList s,bool r ) :
	options( std::move( s ) ),reverseMode( r ),success( true )
{
}

engines::engine::Options::Options( QStringList s ) :
	options( std::move( s ) ),reverseMode( false ),success( true )
{
}

engines::engine::Options::Options( bool r ) :
	reverseMode( r ),success( true )
{
}

engines::engine::Options::Options() : success( false )
{
}

engines::engine::options::options( const favorites::entry& e,const QString& volumeKey ) :
	cipherFolder( e.volumePath ),
	plainFolder( e.mountPointPath ),
	key( volumeKey ),
	idleTimeout( e.idleTimeOut ),
	configFilePath( e.configFilePath ),
	type( QString() ),
	ro( e.readOnlyMode ? e.readOnlyMode.onlyRead() : false ),
	reverseMode( e.reverseMode ),
	mountOptions( e.sanitizedMountOptions() )
{
}

engines::engine::options::options( const QString& cipher_folder,
				   const QString& plain_folder,
				   const QString& volume_key,
				   const QString& idle_timeout,
				   const QString& config_file_path,
				   const QString& volume_type,
				   bool unlock_in_read_only,
				   bool unlock_in_reverse_mode,
				   const QString& mount_options,
				   const QString& create_options ) :
	cipherFolder( cipher_folder ),
	plainFolder( plain_folder ),
	key( volume_key ),
	idleTimeout( idle_timeout ),
	configFilePath( config_file_path ),
	type( volume_type ),
	ro( unlock_in_read_only ),
	reverseMode( unlock_in_reverse_mode ),
	mountOptions( favorites::entry::sanitizedMountOptions( mount_options ) ),
	createOptions( create_options )
{
}
