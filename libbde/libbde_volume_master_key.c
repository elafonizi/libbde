/*
 * Volume Master Key (VMK) metadata entry functions
 *
 * Copyright (C) 2011, Joachim Metz <jbmetz@users.sourceforge.net>
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <byte_stream.h>
#include <memory.h>
#include <types.h>

#include <libcstring.h>
#include <liberror.h>
#include <libnotify.h>

#include "libbde_io_handle.h"
#include "libbde_libfdatetime.h"
#include "libbde_libfguid.h"
#include "libbde_metadata_entry.h"
#include "libbde_stretch_key.h"
#include "libbde_volume_master_key.h"

#include "bde_metadata.h"

uint8_t libbde_volume_master_key_disk_password[ 26 ] = { 'D', 0, 'i', 0, 's', 0, 'k', 0, 'P', 0, 'a', 0, 's', 0, 's', 0, 'w', 0, 'o', 0, 'r', 0, 'd', 0, 0, 0 };

/* Initialize a volume master key
 * Make sure the value volume master key is pointing to is set to NULL
 * Returns 1 if successful or -1 on error
 */
int libbde_volume_master_key_initialize(
     libbde_volume_master_key_t **volume_master_key,
     liberror_error_t **error )
{
	static char *function = "libbde_volume_master_key_initialize";

	if( volume_master_key == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume master key.",
		 function );

		return( -1 );
	}
	if( *volume_master_key == NULL )
	{
		*volume_master_key = memory_allocate_structure(
		                      libbde_volume_master_key_t );

		if( *volume_master_key == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create volume master key.",
			 function );

			goto on_error;
		}
		if( memory_set(
		     *volume_master_key,
		     0,
		     sizeof( libbde_volume_master_key_t ) ) == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear volume master key.",
			 function );

			goto on_error;
		}
	}
	return( 1 );

on_error:
	if( *volume_master_key != NULL )
	{
		memory_free(
		 *volume_master_key );

		*volume_master_key = NULL;
	}
	return( -1 );
}

/* Frees a volume master key
 * Returns 1 if successful or -1 on error
 */
int libbde_volume_master_key_free(
     libbde_volume_master_key_t **volume_master_key,
     liberror_error_t **error )
{
	static char *function = "libbde_volume_master_key_free";

	if( volume_master_key == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume master key.",
		 function );

		return( -1 );
	}
	if( *volume_master_key != NULL )
	{
		memory_free(
		 *volume_master_key );

		*volume_master_key = NULL;
	}
	return( 1 );
}

/* Reads a volume master key from the metadata entry
 * Returns the number of bytes read if successful or -1 on error
 */
int libbde_volume_master_key_read(
     libbde_volume_master_key_t *volume_master_key,
     libbde_io_handle_t *io_handle,
     libbde_metadata_entry_t *metadata_entry,
     liberror_error_t **error )
{
	uint8_t key[ 32 ];

	libbde_metadata_entry_t *property_metadata_entry = NULL;
	libbde_stretch_key_t *stretch_key                = NULL;
	uint8_t *value_data                              = NULL;
	static char *function                            = "libbde_volume_master_key_read";
	size_t value_data_size                           = 0;
	ssize_t read_count                               = 0;
	uint8_t use_recovery_key                         = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	libcstring_system_character_t filetime_string[ 24 ];
	libcstring_system_character_t guid_string[ LIBFGUID_IDENTIFIER_STRING_SIZE ];

	libfdatetime_filetime_t *filetime                = NULL;
	libfguid_identifier_t *guid                      = NULL;
	uint32_t value_32bit                             = 0;
	int result                                       = 0;
#endif

	if( volume_master_key == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid volume master key.",
		 function );

		return( -1 );
	}
	if( metadata_entry == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid metadata entry.",
		 function );

		return( -1 );
	}
	if( metadata_entry->value_type != 0x0008 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: invalid metadata entry - unsupported value type: 0x%04" PRIx16 ".",
		 function,
		 metadata_entry->value_type );

		return( -1 );
	}
	value_data      = metadata_entry->value_data;
	value_data_size = metadata_entry->value_data_size;

	if( value_data_size < 28 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_BOUNDS,
		 "%s: value data size value out of bounds.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	if( libfguid_identifier_initialize(
	     &guid,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create GUID.",
		 function );

		goto on_error;
	}
	if( libfguid_identifier_copy_from_byte_stream(
	     guid,
	     value_data,
	     16,
	     LIBFGUID_ENDIAN_LITTLE,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy byte stream to GUID.",
		 function );

		goto on_error;
	}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
	result = libfguid_identifier_copy_to_utf16_string(
		  guid,
		  (uint16_t *) guid_string,
		  LIBFGUID_IDENTIFIER_STRING_SIZE,
		  error );
#else
	result = libfguid_identifier_copy_to_utf8_string(
		  guid,
		  (uint8_t *) guid_string,
		  LIBFGUID_IDENTIFIER_STRING_SIZE,
		  error );
#endif
	if( result != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_COPY_FAILED,
		 "%s: unable to copy GUID to string.",
		 function );

		goto on_error;
	}
	libnotify_printf(
	 "%s: key identifier\t\t\t\t: %" PRIs_LIBCSTRING_SYSTEM "\n",
	 function,
	 guid_string );

	if( libfguid_identifier_free(
	     &guid,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free GUID.",
		 function );

		goto on_error;
	}
	value_data      += 16;
	value_data_size -= 16;

	if( libfdatetime_filetime_initialize(
	     &filetime,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create filetime.",
		 function );

		goto on_error;
	}
	if( libfdatetime_filetime_copy_from_byte_stream(
	     filetime,
	     value_data,
	     8,
	     LIBFDATETIME_ENDIAN_LITTLE,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to copy filetime from byte stream.",
		 function );

		goto on_error;
	}
#if defined( LIBCSTRING_HAVE_WIDE_SYSTEM_CHARACTER )
	result = libfdatetime_filetime_copy_to_utf16_string(
		  filetime,
		  (uint16_t *) filetime_string,
		  24,
		  LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
		  LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
		  error );
#else
	result = libfdatetime_filetime_copy_to_utf8_string(
		  filetime,
		  (uint8_t *) filetime_string,
		  24,
		  LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
		  LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
		  error );
#endif
	if( result != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_SET_FAILED,
		 "%s: unable to copy filetime to string.",
		 function );

		goto on_error;
	}
	libnotify_printf(
	 "%s: unknown time\t\t\t\t: %" PRIs_LIBCSTRING_SYSTEM " UTC\n",
	 function,
	 filetime_string );

	if( libfdatetime_filetime_free(
	     &filetime,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free filetime.",
		 function );

		goto on_error;
	}
	value_data      += 8;
	value_data_size -= 8;

	byte_stream_copy_to_uint32_little_endian(
	 value_data,
	 value_32bit );
	libnotify_printf(
	 "%s: unknown1\t\t\t\t\t: 0x%08" PRIx32 "\n",
	 function,
	 value_32bit );

	value_data      += 4;
	value_data_size -= 4;

	libnotify_printf(
	 "\n" );

	while( value_data_size >= sizeof( bde_metadata_entry_v1_t ) )
	{
		if( libbde_metadata_entry_initialize(
		     &property_metadata_entry,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create property metadata entry.",
			 function );

			goto on_error;
		}
		read_count = libbde_metadata_entry_read(
			      property_metadata_entry,
			      value_data,
			      value_data_size,
			      error );

		if( read_count == -1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read property metadata entry.",
			 function );

			goto on_error;
		}
		value_data      += read_count;
		value_data_size -= read_count;

		if( property_metadata_entry->value_type == LIBBDE_VALUE_TYPE_STRING )
		{
			if( libbde_metadata_entry_read_string(
			     property_metadata_entry,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read string from property metadata entry.",
				 function );

				goto on_error;
			}
			if( property_metadata_entry->value_data_size == 26 )
			{
				if( memory_compare(
				     property_metadata_entry->value_data,
				     libbde_volume_master_key_disk_password,
				     26 ) == 0 )
				{
					use_recovery_password = 1;
				}
			}
		}
		else if( property_metadata_entry->value_type == LIBBDE_VALUE_TYPE_STRETCH_KEY )
		{
			if( libbde_stretch_key_initialize(
			     &stretch_key,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create stretch key.",
				 function );

				goto on_error;
			}
			if( libbde_stretch_key_read(
			     stretch_key,
			     io_handle,
			     property_metadata_entry,
			     use_recovery_key,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read stretch key metadata entry.",
				 function );

				libbde_stretch_key_free(
				 &stretch_key,
				 NULL );

				goto on_error;
			}
			if( libbde_stretch_key_free(
			     &stretch_key,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free stretch key.",
				 function );

				goto on_error;
			}
		}
		else if( property_metadata_entry->value_type == LIBBDE_VALUE_TYPE_AES_CMM_ENCRYPTED_KEY )
		{
			if( libbde_metadata_entry_read_aes_ccm_encrypted_key(
			     property_metadata_entry,
			     key,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read AES-CMM encrypted key from property metadata entry.",
				 function );

				goto on_error;
			}
		}
		if( libbde_metadata_entry_free(
		     &property_metadata_entry,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free property metadata entry.",
			 function );

			goto on_error;
		}
	}
#endif
#if defined( HAVE_DEBUG_OUTPUT )
	if( libnotify_verbose != 0 )
	{
		if( value_data_size > 0 )
		{
			libnotify_printf(
			 "%s: trailing data:\n",
			 function );
			libnotify_print_data(
			 value_data,
			 value_data_size );
		}
	}
#endif
	return( 1 );

on_error:
#if defined( HAVE_DEBUG_OUTPUT )
	if( guid != NULL )
	{
		libfguid_identifier_free(
		 &guid,
		 NULL );
	}
	if( filetime != NULL )
	{
		libfdatetime_filetime_free(
		 &filetime,
		 NULL );
	}
#endif
	if( property_metadata_entry != NULL )
	{
		libbde_metadata_entry_free(
		 &property_metadata_entry,
		 NULL );
	}
	return( -1 );
}

