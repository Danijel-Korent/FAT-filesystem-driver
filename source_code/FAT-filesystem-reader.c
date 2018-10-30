
#include <stdint.h>
#include <stdio.h>   //printf, fgets
#include <string.h>  //strcmp


#include "../fat_images/FAT12_3-clusters-clean.h" // Here is located an array of file system binary image


static inline uint16_t read_16(const unsigned char *buffer, int offset);
static inline uint32_t read_32(const unsigned char *buffer, int offset);
static inline uint8_t  read__8(const unsigned char *buffer, int offset); // This one is really here just for uniformity and nicer looking code

    // Info:
    // https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system
    // https://social.technet.microsoft.com/wiki/contents/articles/6771.the-fat-file-system.aspx


    // COPY-PASTER:
    //      - Generate smallest possible FAT12 file system (!! Needs modification in mkfs.fat source to work! )
    //          - 3k total size, with 3 clusters * 512bytes = 1536bytes free space
    //          - works in linux (r/w), can read on windows10, but sometimes crashes explorer.exe on write
    //
    //          /home/danijel/fat_proj/dosfstools/src/mkfs.fat -vvv -F 12 -f 1 -r 16 -R 1 -n POKUS -s 1 -C FAT12_3-clusters-clean 3 > FAT12_3-clusters-clean_info.txt
    //

    /*************************************************************************************
    * OVERALL STRUCTURE OF FAT FS
    *
    *  RESERVED SECTORS
    *      - VBR - Volume boot record / partition boot sector (1sector/512bytes)
    *      - FS Information Sector (FAT32 only)
    *      - Rest of reserved sectors
    *  FAT TABLE #1
    *  FAT TABLE #2        (optional)
    *  ROOT DIRECTORY AREA (FAT12/16 only)
    *  DATA AREA
    *       - Cluser_01
    *       - Cluser_xy -> until the end of partition
    *
    */

    /*************************************************************************************
    * OVERALL STRUCTURE OF Volume Boot Record (VBR) - the first 512 bytes of a FS (not MBR)
    *
    *     0 -   2  Boot code jump instruction / EB 3C 90 -> jmp 0x3e, nop
    *     3 -   8  OEM Name string
    *     9 -  35  BIOS Parametar block (holds important FAT parameters)(common to all FAT variants??)
    *    36 -  61  Extended BIOS Parameter Block
    *
    *       36 -  61  FAT 12/16 specific data of VBR
    *       62 - 510  Boot code (bootloader)   --> Sources conficting, but generated boot jump instruction for FAT12 jumps to 0x3e
    *
    *       36 -  89  FAT 32 specific data of VBR   ???
    *       90 - 510  Boot code (bootloader)        ???
    *
    *   511 - 512 - Boot sector signature
    */

// TODO NEXT: Add interface for accesing directory items and reading files

// Okay, so we now know how to find data and how the data is encoded, time to implement interfaces
typedef struct directory_handle_tag
{
    uint32_t first_cluster_no;
    uint32_t seek;
} directory_handle_t;

#define MAX_NAME_SIZE (11)

typedef struct directory_entry_tag
{
    uint8_t name[MAX_NAME_SIZE + 1];
    uint8_t type;
    uint32_t size;
} directory_entry_t;

typedef struct file_handle_tag
{
    uint32_t first_cluster_no;
    uint32_t size;
    uint32_t seek;
} file_handle_t;

enum
{
    e_SUCCESS,
    e_FILE_NOT_FOUND,
    e_END_OF_DIR
}
e_fatErrorCodes;

enum
{
    e_FILE,
    e_DIRECTORY
}
e_dirEntryType;

// File system is read-only and there will bi no internal states or allocated buffers, s
// so no point in open and close functions...

int8_t find_directory            ( directory_handle_t* const handle, const uint8_t* const path);
int8_t read_next_directory_entry ( directory_handle_t* const handle, directory_entry_t* const dir_entry );

int8_t find_file( file_handle_t* const handle, const uint8_t* const path );
int8_t file_read( file_handle_t* const handle, uint8_t* const buffer, const uint32_t buffer_size, uint32_t* const successfully_read);


static void print_root_files(void)
{
    // TODO: put all necessary logic here, and then split it into functional parts later


    // Offsets of the Volume Boot Record (VBR)
    // - BIOS Parametar block - Common data for FAT 12/16/32 (up to 0x23)
    const uint_fast8_t dummy                    = 0x00;
    const uint_fast8_t JUMP_INSTRUCTION_1       = 0x00;
    const uint_fast8_t JUMP_INSTRUCTION_2       = 0x01;
    const uint_fast8_t JUMP_INSTRUCTION_3       = 0x02;
    const uint_fast8_t OEM_NAME_8_BYTE          = 0x03; // Not null-terminated
    const uint_fast8_t BYTES_PER_SECTOR_16b     = 0x0b;
    const uint_fast8_t SECTORS_PER_CLUSTER      = 0x0d;
    const uint_fast8_t NUM_RESERVED_SECTORS_16b = 0x0e;
    const uint_fast8_t NUM_OF_FATS              = 0x10;

    const uint_fast8_t MAX_NUM_OF_ROOT_DIR_16b  = 0x11; // Only FAT12/16

    const uint_fast8_t NUM_OF_TOTAL_SECTORS_16b = 0x13; // Used if partition is smaler than 32MB
    const uint_fast8_t MEDIA_DESCRIPTOR         = 0x15;

    const uint_fast8_t SECTORS_PER_ALLOC_TBL_16b = 0x16; // Only FAT12/16
    const uint_fast8_t SECTORS_PER_TRACK_16b     = 0x18;
    const uint_fast8_t NUM_OF_HEADS_16b          = 0x1a;
    const uint_fast8_t HIDDEN_SECTORS_32b        = 0x1c;
    const uint_fast8_t NUM_OF_TOTAL_SECTORS_32b  = 0x20; // If NUM_OF_TOTAL_SECTORS_16b is zero, this one is used


    const unsigned char* const FS_image = FAT12_3_clusters_clean;


    // Print the BIOS Parametar block settings
    char oem_string[8 + 1] = {0};
    strncpy(oem_string, (const char*)(FS_image + OEM_NAME_8_BYTE), sizeof(oem_string)-1);

    printf("\n JUMP_INSTRUCTION_1:   %#x", read__8(FS_image, JUMP_INSTRUCTION_1));
    printf("\n JUMP_INSTRUCTION_2:   %#x", read__8(FS_image, JUMP_INSTRUCTION_2));
    printf("\n OEM:                  %s",  oem_string);
    printf("\n BYTES_PER_SECTOR:     %i",  read_16(FS_image, BYTES_PER_SECTOR_16b));
    printf("\n SECTORS_PER_CLUSTER:  %i",  read__8(FS_image, SECTORS_PER_CLUSTER));
    printf("\n NUM_RESERVED_SECTORS: %i",  read_16(FS_image, NUM_RESERVED_SECTORS_16b));
    printf("\n NUM_OF_FATS:          %i",  read__8(FS_image, NUM_OF_FATS));


    // TODO: CLEAN UP THIS MESS
    // Offsets of the Volume Boot Record (VBR)
    // - Extended BIOS Parametar block - FAT 12/16 ONLY !!!
    //  8b Number of drives
    //  8b Reserved
    //  8b Extended boot signature, always 0x29
    // 32b Volume ID
    // 88b Volume label string (11 bytes)
    // 80b File system type (8 bytes)
    const uint_fast8_t FAT16_BOOT_SIGN_8b     = 0x26; // Extended boot signature, always 0x29
    const uint_fast8_t FAT16_VOLUME_ID_32b    = 0x27;
    const uint_fast8_t FAT16_VOLUME_LABEL_88b = 0x2B;
    const uint_fast8_t FAT16_FS_TYPE_64b      = 0x36;

    // Offsets of the Volume Boot Record (VBR)
    // - Extended BIOS Parametar block - FAT 32 ONLY !!!
    const uint_fast8_t FAT32_SECTORS_PER_FAT_32b      = 0x24; // also named "FAT Size"
    const uint_fast8_t FAT32_EXTERNAL_FLAGS_16b       = 0x28; // ??? check if this is correct
    const uint_fast8_t FAT32_FILE_SYSTE_VERSION_16b   = 0x2A; // ??? check if this is correct
    const uint_fast8_t FAT32_ROOT_DIR_1ST_CLUSTER_32b = 0x2c;
    // 16b File system info
    // 16b Boot record backup
    // 96b of reserved space
    //  8b Number of drives
    //  8b Reserved
    //  8b Extended boot signature
    // 32b Volume ID
    // 88b Volume label string (11 bytes)
    // 80b File system type (8 bytes)


    // Print the extened BIOS Parametar block settings - for FAT12 variant
    char label[11 + 1]  = {0};
    strncpy(label, (const char*)(FS_image + FAT16_VOLUME_LABEL_88b), sizeof(label)-1);

    char fs_type[8 + 1] = {0};
    strncpy(fs_type, (const char*)(FS_image+ FAT16_FS_TYPE_64b), sizeof(fs_type)-1);

    printf("\n FAT16_VOLUME_LABEL:   %s",label);
    printf("\n FAT16_FS_TYPE:        %s",fs_type);

    printf("\n Boot block signature: %#x %#x", FS_image[510], FS_image[511]);


    // Offsets for directory entry structure
    const uint_fast8_t file_name_64b        = 0x00;
    const uint_fast8_t file_extension_24b   = 0x08;
    const uint_fast8_t file_attributes_8b   = 0x0b;
    const uint_fast8_t file_1st_cluster_16b = 0x1a;
    const uint_fast8_t file_size_32b        = 0x1c;

    // TODO: hardcoded at the moment - calculate it from the VBR data
    const uint8_t* const root_dir_base =  FS_image + 1024;
    const uint_fast8_t directory_slots_num = 16;

    // Iterate the root directory entries and print the parameters
    printf("\n\n --> Content of the root directory");

    for( int i = 0; i < directory_slots_num; i++ )
    {
        const uint8_t* const directory_entry_base = root_dir_base + i*32; // 32 is the size of the directory entry structure

        // First byte in file name have special meaning. If zero - slot is unused
        if( 0 == *directory_entry_base )
        {
            break; // Rest of the slots are also empty according to the specs
        }

        char file_name[8+3 + 1]  = {0};
        strncpy(file_name, (const char*)(directory_entry_base), sizeof(file_name)-1); // Get both the name and extension

        printf("\n\n ENTRY NO.%i", i);
        printf("\n   File name:   %s", file_name);
        printf("\n   File attributes:   %#x", read__8(directory_entry_base, file_attributes_8b));
        printf("\n   First cluster:     %#x", read_16(directory_entry_base, file_1st_cluster_16b));
        printf("\n   File size:         %i",  read_32(directory_entry_base, file_size_32b));
    }

    // TODO NEXT:
    //      - Print the content of the directories
    //      - Print the content of the files

    // Experimental reading of directory content
    // TEMP HARDCODED: Print a content of a directory DIR_1 of current image
    // TODO: currently hardcoded, modify to calculate and iterate all directories
    // Iterate the directory entries and print the parameters
    {
        printf("\n\n --> Content of the DIR_1 directory");

        const uint8_t* const clusters_start_addr = root_dir_base + 512; // In this image clusters start in 1 sector after "root directory" area

        for( int i = 0; i < directory_slots_num; i++ )
        {
            const uint8_t* const directory_entry_base = clusters_start_addr
                                                        + 512   // Dir_1 is located in 2nd cluster
                                                        + i*32; // 32 is the size of the directory entry structure

            // First byte in file name have special meaning. If zero - slot is unused
            if( 0 == *directory_entry_base )
            {
                break; // Rest of the slots are also empty according to the specs
            }

            char file_name[8+3 + 1]  = {0};
            strncpy(file_name, (const char*)(directory_entry_base), sizeof(file_name)-1); // Get both the name and extension

            printf("\n\n ENTRY NO.%i", i);
            printf("\n   File name:   %s", file_name);
            printf("\n   File attributes:   %#x", read__8(directory_entry_base, file_attributes_8b));
            printf("\n   First cluster:     %#x", read_16(directory_entry_base, file_1st_cluster_16b));
            printf("\n   File size:         %i",  read_32(directory_entry_base, file_size_32b));
        }
    }

    // Experimental reading of file content
    // TEMP HARDCODED: Print a content of a file FILE_1 of current image
    {
        const uint8_t* const clusters_start_addr = root_dir_base + 512; // In this image clusters start in 1 sector after "root directory" area

        const uint8_t* const file_start = clusters_start_addr + 2 * 512; // cluster_start + cluster no. * cluster size

        const int file_size = 18; // HARDCODED to FILE_1

        uint8_t string_buffer[512] = {0}; // One cluster is one sector in our case - 512 bytes, could be bigger so this need to be calculated

        memcpy(string_buffer, file_start, file_size);

        puts("\n\n --> Content of the file FILE_1");
        puts(string_buffer);
    }
}


int8_t find_directory ( directory_handle_t* const handle, const uint8_t* const path)
{
    // TODO: implement, basically just find the first cluster
#if 1
    handle->first_cluster_no = 0; // Zero is invalid value
    handle->seek = 0;

    // Dummy stub for testing

    static uint8_t dummy_directory_names[][100] =
    {
        {"/"},
        {"/home/"},
        {"/home/user/"}
    };

    for( int i = 0; i < sizeof(dummy_directory_names)/sizeof(dummy_directory_names[0]); i++)
    {
        if( 0 == strcmp(path, dummy_directory_names[i]) )
        {
            handle->first_cluster_no = i + 1;
            return e_SUCCESS;
        }
    }
#endif

    return e_FILE_NOT_FOUND;
}

int8_t read_next_directory_entry ( directory_handle_t* const handle, directory_entry_t* const dir_entry )
{
#if 1
    // Dummy stub for testing

    static directory_entry_t dummy_entries[][3] =
    {
        {
            // root
            {"home",   e_DIRECTORY,        0},
            {"root_file01", e_FILE,        0},
            {"root_file02", e_FILE,       10}
        },
        {
            // home
            //{".",     e_DIRECTORY,   0},
            //{"..",    e_DIRECTORY,   0},
            {"user",   e_DIRECTORY,        0},
            {"home_file11", e_FILE,        0},
            {"home_file12", e_FILE,       11}
        },
        {
            //user
            {"user_file21", e_FILE,        0},
            {"user_file22", e_FILE,        0},
            {"user_file23", e_FILE,       12}
        }
    };

    if( handle->seek < sizeof(dummy_entries[0])/sizeof(dummy_entries[0][0]) )
    {
        size_t index = handle->first_cluster_no - 1;

        size_t table_size = sizeof(dummy_entries) / sizeof(dummy_entries[0]);

        if( index < table_size )
        {
            *dir_entry = dummy_entries[index][handle->seek++];

            return e_SUCCESS;
        }
    }
#else
    // TODO: implement -> fetch the entry data using cluster number and seek

#endif
    return e_END_OF_DIR;
}


/***********************************************************************************************************************
 *                                            PSEUDO SHELL SECTION                                                     *
 ***********************************************************************************************************************/
static void execute_command_cd(uint8_t* const args, const uint32_t args_lenght);
static void execute_command_ls(uint8_t* const args, const uint32_t args_lenght);

#define MAX_PATH_SIZE (100 + 1) // 100 bytes should be enough for everybody!
#define MAX_INPUT_LEN (100 + 1)

uint8_t shell_pwd[MAX_PATH_SIZE]  = "/";


static void run_pseudo_shell(void)
{
    // TODO: Implement a simple shell-like interface that supports cd, ls and cat commands

    uint8_t user_input[MAX_INPUT_LEN] = {0};

    printf("\n\n\nshell:%s $ ", shell_pwd);

    while (NULL != fgets(user_input, sizeof(user_input), stdin))
    {
         // NUll-terminate input, just in case
        user_input[sizeof(user_input)-1] = 0;

        // A workaround because fgets include a newline in a string
        int length = strlen(user_input);
        if( length > 0 )
        {
            if( '\n' == user_input[length-1] ) user_input[length-1] = 0;
        }

        // Process input
        if (0 == strcmp(user_input, "exit"))
        {
            break;
        }
        else if ('q' == user_input[0] && 0 == user_input[1])
        {
            break;
        }
        else if (0 == user_input[0])
        {
            // Empty input - do nothing
        }
        else if ('c' == user_input[0] && 'd' == user_input[1]) // cd
        {
            uint8_t *args      = NULL;
            uint32_t args_len  = 0;

            size_t current_input_len = strlen(user_input) + 1; // + null-terminator

            // TODO: duplicated code
            if( current_input_len > (3 + 1) )
            {
                args     = user_input + 3;
                args_len = current_input_len - 3;
            }

            execute_command_cd(args, args_len);
        }
        else if ('l' == user_input[0] && 's' == user_input[1]) // ls
        {
            execute_command_ls(NULL, 0);
        }
        else
        {
            printf("%s: Unknown command\n", user_input);
        }

        printf("\nshell:%s $ ", shell_pwd);
    }
}

static void execute_command_cd(uint8_t* const args, const uint32_t args_lenght)
{
#if 1
    // TODO: validate/sanitize input args, currently PWD is just updated without any checks

    if( NULL == args || 0 == args_lenght)
    {
        return; // Nothing to do here, maybe print no args..
    }

    // Save the old pwd
    uint8_t old_pwd[MAX_PATH_SIZE]  = "";
    strncpy(old_pwd, shell_pwd, MAX_PATH_SIZE);

    // NUll-terminate, just in case
    args[args_lenght-1] = 0;

    // Update the shell pwd to new path
    if( '/' == args[0] )
    {
        // If full path just copy the argument
        strncpy( shell_pwd, args, sizeof(shell_pwd) - 1);
    }
    else
    {
        // If relative path - append argument to current pwd
        size_t current_pwd_len   = strlen(shell_pwd); // Without null-term, but args_lenght already inludes null-terminator size
        size_t max_available_len = sizeof(shell_pwd); // With null termn

        if( ( current_pwd_len + args_lenght) < max_available_len )
        {
            strcat(shell_pwd, args);
        }

        current_pwd_len = strlen(shell_pwd);
    }

    // Append '/' at the end if it is not there
    size_t last_char_index = strlen(shell_pwd) - 1;

    if( '/' != shell_pwd[last_char_index])
    {
        strcat(shell_pwd, "/"); // TODO: check if there is enought space for this
    }

    // If directory in updated shell pwd cannot be found, restore old pwd
    directory_handle_t handle;
    if( e_SUCCESS != find_directory(&handle, shell_pwd) )
    {
        printf("cd: directory \"%s\" not found!", shell_pwd);
        strncpy(shell_pwd, old_pwd, MAX_PATH_SIZE);
    }
#else
    printf("cd unimplemented");
#endif
}

static void execute_command_ls(uint8_t* const args, const uint32_t args_lenght)
{
#if 1
    if( 0 != args_lenght)
    {
        args[args_lenght-1] = 0; // NUll-terminate, just in case
    }
    else
    {
        // TODO: replace args with PWD
    }

    directory_handle_t dir_handle;
    directory_entry_t  dir_entry;

    find_directory(&dir_handle, args);

    printf("\n TYPE   SIZE  NAME  \n");

    while( e_END_OF_DIR != read_next_directory_entry( &dir_handle, &dir_entry ) )
    {
        uint8_t *type = "file";

        if( e_FILE == dir_entry.type )
        {
            type = " dir";
        }

        printf(" %s %6i  %s \n", type, dir_entry.size , dir_entry.name);
    }
#else
    printf(" ls example output: \n\n");

    printf(" %s %06i  %s \n", "d", 0, "DIR_5");
    printf(" %s %06i  %s \n", "f", 5, "FILE_6");
    printf(" %s %06i  %s \n", "f", 255, "FILE_7");
#endif
}

int main(void)
{
    print_root_files();

    run_pseudo_shell();

    return 0;
}

/***********************************************************************************************************************
 *                                           HELPER FUNCTIONS                                                          *
 ***********************************************************************************************************************/

static inline uint16_t read_16( const unsigned char *buffer, int offset)
{
    return (buffer[offset+1] << 8) + buffer[offset];
}

static inline uint32_t read_32(const unsigned char *buffer, int offset)
{
    return (buffer[offset + 3] << 24) + (buffer[offset + 2] << 16) + (buffer[offset + 1] << 8) + buffer[offset];
}

// This one is really here just for uniformity and nicer looking code
static inline uint8_t read__8(const unsigned char *buffer, int offset)
{
    return buffer[offset];
}
