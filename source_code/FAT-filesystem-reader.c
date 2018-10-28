
#include<stdint.h>
#include<stdio.h>   //printf, gets_s
#include<string.h>  //strcmp


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


static void print_root_files(void)
{
    // TODO: put all necessary logic here, and then split it into functional parts


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


    // Fetch the root directory area (FAT12/16 only)

    // Print the entries/files from root directory
    //      - (temp) hardcode manualy the offset of the root dir
    //      - Add the offsets for the "directory entry"
    //      - Iterate over all slots
    //      - Check if it is active - If active, print parameters
    //      - calculate the offset of the root dir


    const uint_fast8_t file_name_64b        = 0x00;
    const uint_fast8_t file_extension_24b   = 0x08;
    const uint_fast8_t file_attributes_8b   = 0x0b;
    const uint_fast8_t file_1st_cluster_16b = 0x1a;
    const uint_fast8_t file_size_32b        = 0x1c;

    // TODO: hardcoded at the moment - calculate it from the VBR data
    const uint8_t* const root_dir_base =  FS_image + 1024;
    const uint_fast8_t directory_slots_num = 16;

    // Iterate the root directory entries and print the parameters
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


    // Example output of the function
    printf("\n\n\n Funtion \"print_root_files\" example output: \n\n");

    printf(" TYPE SIZE NAME  \n");
    printf("  dir    0  Dir1  \n");
    printf("  dir    0  Dir2  \n");
    printf(" file   22  File1 \n");
}

/***********************************************************************************************************************
 *                                            PSEUDO SHELL SECTION                                                     *
 ***********************************************************************************************************************/
static void execute_command_cd(const char *args, int args_lenght);
static void execute_command_ls(const char *args, int args_lenght);


static void run_pseudo_shell(void)
{
    // TODO: Implement a simple shell-like interface that supports cd, ls and cat commands

    char user_input[100] = {0};
    char pwd[100]        = "/";

    printf("\n\n\nshell:%s$ ", pwd);

    while (NULL != gets_s(user_input, sizeof(user_input)))
    {
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
            execute_command_cd(NULL, 0);
        }
        else if ('l' == user_input[0] && 's' == user_input[1]) // ls
        {
            execute_command_ls(NULL, 0);
        }
        else
        {
            printf("%s: Unknown command", user_input);
        }

        printf("\nshell:%s$ ", pwd);
    }
}

static void execute_command_cd(const char *args, int args_lenght)
{
    printf("cd unimplemented");
}

static void execute_command_ls(const char *args, int args_lenght)
{
    printf(" ls example output: \n\n");

    printf(" %s %06i  %s \n", "d", 0, "DIR_5");
    printf(" %s %06i  %s \n", "f", 5, "FILE_6");
    printf(" %s %06i  %s \n", "f", 255, "FILE_7");
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
