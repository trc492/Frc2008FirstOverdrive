#if 0
/// <module name="ifi_startup.c" />
///
/// <summary>
///     This module contains startup code.
/// </summary>
///
/// <remarks>
///     Modified: Michael Tsang (mikets@hotmail.com)
///     Date:     03-Feb-2008
///     Based on FRC version.
///     Environment: MPLAB
/// </remarks>
#endif

//
// Local constants.
//
#define RND                     6

//
// Local type definitions.
//

// MPLAB-C18 initialized data memory support.
// The linker will populate the _cinit table.
extern far rom struct
{
    unsigned short num_init;
    struct _init_entry
    {
        unsigned long from;
        unsigned long to;
        unsigned long size;
    } entries[];
} _cinit;

//
// Local function prototypes.
//
void
_entry(
    void
    );

void
_startup(
    void
    );

void
_do_cinit(
    void
    );

//
// Imported function prototypes.
//
extern void
Clear_Memory(
    void
    );

extern void
main(
    void
    );

//
// Imported data locations.
//
extern volatile near unsigned long short TBLPTR;
extern near unsigned FSR0;
extern near char FPFLAGS;

#pragma code _entry_scn=RESET_VECTOR

/// <summary>
///     This is the main entry point. After the processor is reset, it
///     jumps to the code located at RESET_VECTOR which is where this
///     code is located. This start up location contains only one
///     instruction. It immediately jumps to the _startup function
///     which is located in the _startup code section.
/// </summary>
///
/// <returns> None. </returns>

void
_entry (
    void
    )
{
    _asm
        goto _startup
    _endasm
}       //_entry

#pragma code _startup_scn

/// <summary>
///     This is the startup function. It initializes the processor
///     environment including setting up the stack and processor
///     flags. Then it goes into a loop clearing memory, copying
///     ROM to memory blocks and executing user's main code.
/// </summary>
///
/// <returns> None. </returns>

void
_startup(
    void
    )
{
    _asm
        //Initialize the stack pointer.
        lfsr    1,_stack
        lfsr    2,_stack
        clrf    TBLPTRU,0       //1st silicon doesn't do this on POR.
        bcf     FPFLAGS,RND,0   //Initialize rounding flag for floating
                                //point libs.
        //
        // Initialize the flash memory access configuration. this is harmless
        // for non-flash devices, so we do it on all parts.
        //
        bsf     0xa6,7,0
        bcf     0xa6,6,0
    _endasm

loop:
    Clear_Memory();
    _do_cinit();
    //Call the user's main routine.
    main();
    goto loop;
}       //_startup

#pragma code _cinit_scn

/// <summary>
///     This function copies the ROM to memory blocks according to the
///     _ciniit table.
/// </summary>
///
/// <returns> None. </returns>

void
_do_cinit(
    void
    )
{
    //
    // We'll make the assumption in the following code that these statics
    // will be allocated into the same bank.
    //
    static short long prom;
    static unsigned short curr_byte;
    static unsigned short curr_entry;
    static short long data_ptr;

    // Initialized data...
    TBLPTR = (short long)&_cinit;
    _asm
        movlb   data_ptr
        tblrdpostinc
        movf    TABLAT,0,0
        movwf   curr_entry,1
        tblrdpostinc
        movf    TABLAT,0,0
        movwf   curr_entry+1,1
    _endasm
test:
    _asm
        bnz     3
        tstfsz  curr_entry,1
        bra     1
    _endasm
    goto done;
    //
    // Count down so we only have to look up the data in _cinit once.
    // At this point we know that TBLPTR points to the top of the current
    // entry in _cinit, so we can just start reading the from, to, and
    // size values.
    //
    _asm
        // Read the source address.
        tblrdpostinc
        movf    TABLAT,0,0
        movwf   prom,1
        tblrdpostinc
        movf    TABLAT,0,0
        movwf   prom+1,1
        tblrdpostinc
        movf    TABLAT,0,0
        movwf   prom+2,1
        // Skip a byte since it's stored as a 32bit int.
        tblrdpostinc
        // Read the destination address directly into FSR0.
        tblrdpostinc
        movf    TABLAT,0,0
        movwf   FSR0L,0
        tblrdpostinc
        movf    TABLAT,0,0
        movwf   FSR0H,0
        // Skip two bytes since it's stored as a 32bit int.
        tblrdpostinc
        tblrdpostinc
        // Read the destination address directly into FSR0.
        tblrdpostinc
        movf    TABLAT,0,0
        movwf   curr_byte,1
        tblrdpostinc
        movf    TABLAT,0,0
        movwf   curr_byte+1,1
        // Skip two bytes since it's stored as a 32bit int.
        tblrdpostinc
        tblrdpostinc
    _endasm
    //
    // The table pointer now points to the next entry. Save it
    // off since we'll be using the table pointer to do the copying
    // for the entry.
    //
    data_ptr = TBLPTR;

    // Now assign the source address to the table pointer.
    TBLPTR = prom;

    // Do the copy loop.
    _asm
        // Determine if we have any more bytes to copy.
        movlb   curr_byte
        movf    curr_byte,1,1
copy_loop:
        bnz     2               // goto copy_one_byte
        movf    curr_byte+1,1,1
        bz      7               // goto done_copying

copy_one_byte:
        tblrdpostinc
        movf    TABLAT,0,0
        movwf   POSTINC0,0

        // Decrement byte counter
        decf    curr_byte,1,1
        bc      -8              // goto copy_loop
        decf    curr_byte+1,1,1
        bra     -7              // goto copy_one_byte

done_copying:
    _endasm
    // Restore the table pointer for the next entry.
    TBLPTR = data_ptr;
    // Next entry...
    curr_entry--;
    goto test;
done:
    ;
}       //_do_cinit
