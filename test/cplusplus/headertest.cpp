// purpose of this test is to find out if our headers are C++ compatible

// net headers need this
#define __BSD_VISIBLE 1

// get rid of a dbus error
#define DBUS_API_SUBJECT_TO_CHANGE

#include <hardware/acpi/acpi.h>

// ADF redefines AROS types
//#include <adf/adf_blk.h>
//#include <adf/adf_defs.h>
//#include <adf/adf_err.h>
//#include <adf/adf_hd.h>
//#include <adf/adf_str.h>
//#include <adf/adflib.h>
//#include <adf/defendian.h>
#include <alloca.h>
#include <aros/64bit.h>
//#include <aros/arm/_fpmath.h>
//#include <aros/arm/atomic_v6.h>
//#include <aros/arm/atomic_v7.h>
//#include <aros/arm/atomic.h>
//#include <aros/arm/cpu.h>
//#include <aros/arm/cpucontext.h>
//#include <aros/arm/fenv.h>
#include <aros/arosbase.h>
#include <aros/arossupportbase.h>
#include <aros/asmcall.h>
#include <aros/atomic.h>
#include <aros/autoinit.h>
#include <aros/bigendianio.h>
#include <aros/bootloader.h>
#include <aros/build.h>
#include <aros/config.h>
#include <aros/cpu.h>
#include <aros/debug.h>
#include <aros/detach.h>
//#include <aros/i386/_fpmath.h>
//#include <aros/i386/atomic.h>
//#include <aros/i386/cpu.h>
//#include <aros/i386/cpucontext.h>
//#include <aros/i386/fenv.h>
#include <aros/inquire.h>
#include <aros/io.h>
#include <aros/kernel.h>
#include <aros/libcall.h>
#include <aros/locale.h>
//#include <aros/m68k/_fpmath.h>
//#include <aros/m68k/atomic.h>
//#include <aros/m68k/cpu.h>
//#include <aros/m68k/cpucontext.h>
//#include <aros/m68k/fenv.h>
//#include <aros/m68k/fpucontext.h>
//#include <aros/m68k/libcall.h>
#include <aros/macros.h>
#include <aros/mathieee64bitdefines.h>
//#include <aros/morphos/cpu.h>
#include <aros/multiboot.h>
#include <aros/oldprograms.h>
//#include <aros/ppc/_fpmath.h>
//#include <aros/ppc/atomic.h>
//#include <aros/ppc/cpu.h>
//#include <aros/ppc/cpucontext.h>
//#include <aros/ppc/fenv.h>
#include <aros/purify.h>
#include <aros/rt.h>
#include <aros/shcommands_embedded.h>
#include <aros/shcommands_notembedded.h>
#include <aros/shcommands.h>
#include <aros/startup.h>
#include <aros/symbolsets.h>
#include <aros/system.h>
#include <aros/systypes.h>
#include <aros/types/blk_t.h>
#include <aros/types/clock_t.h>
#include <aros/types/clockid_t.h>
#include <aros/types/dev_t.h>
#include <aros/types/fs_t.h>
#include <aros/types/gid_t.h>
#include <aros/types/id_t.h>
#include <aros/types/ino_t.h>
#include <aros/types/int_t.h>
#include <aros/types/intptr_t.h>
#include <aros/types/iovec_s.h>
#include <aros/types/itimerspec_s.h>
#include <aros/types/key_t.h>
#include <aros/types/mbstate_t.h>
#include <aros/types/mode_t.h>
#include <aros/types/nlink_t.h>
#include <aros/types/null.h>
#include <aros/types/off_t.h>
#include <aros/types/pid_t.h>
#include <aros/types/ptrdiff_t.h>
#include <aros/types/regoff_t.h>
#include <aros/types/seek.h>
#include <aros/types/sigaction_s.h>
#include <aros/types/sigevent_s.h>
#include <aros/types/siginfo_t.h>
#include <aros/types/sigset_t.h>
#include <aros/types/size_t.h>
#include <aros/types/socklen_t.h>
#include <aros/types/ssize_t.h>
#include <aros/types/stack_t.h>
#include <aros/types/suseconds_t.h>
#include <aros/types/time_t.h>
#include <aros/types/timer_t.h>
#include <aros/types/timespec_s.h>
#include <aros/types/timeval_s.h>
#include <aros/types/ucontext_t.h>
#include <aros/types/uid_t.h>
#include <aros/types/uintptr_t.h>
#include <aros/types/useconds_t.h>
#include <aros/types/wchar_t.h>
#include <aros/types/wint_t.h>
//#include <aros/x86_64/_fpmath.h>
//#include <aros/x86_64/atomic.h>
//#include <aros/x86_64/cpu.h>
//#include <aros/x86_64/cpucontext.h>
//#include <aros/x86_64/fenv.h>
#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <arpa/telnet.h>
#include <arpa/tftp.h>
//#include <asm/arm/cpu.h>
//#include <asm/arm/mx51.h>
#include <asm/cpu.h>
//#include <asm/i386/cpu.h>
//#include <asm/i386/io.h>
#include <asm/io.h>
//#include <asm/ppc/io.h>
//#include <asm/x86_64/cpu.h>
#include <assert.h>

#include <bluetooth/assignednumbers.h>
#include <bluetooth/avdtp.h>
#include <bluetooth/hci.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>

#include <bsdsocket/socketbasetags.h>
#include <bsdsocket/types.h>
#include <bzlib.h>

// disabled because it redefines some AROS types
//#include <c_iff.h>

#include <complex.h>
#include <ctype.h>
#include <cybergraphx/cgxvideo.h>
#include <cybergraphx/cybergraphics.h>

#include <datatypes/amigaguideclass.h>
#include <datatypes/animationclass.h>
#include <datatypes/datatypes.h>
#include <datatypes/datatypesclass.h>
#include <datatypes/pictureclass.h>
#include <datatypes/PictureClassExt.h>
#include <datatypes/soundclass.h>
#include <datatypes/soundclassext.h>
#include <datatypes/textclass.h>

// only <dbus/dbus.h> can be included
//#include <dbus/dbus-address.h>
//#include <dbus/dbus-arch-deps.h>
//#include <dbus/dbus-bus.h>
//#include <dbus/dbus-connection.h>
//#include <dbus/dbus-errors.h>
//#include <dbus/dbus-macros.h>
//#include <dbus/dbus-memory.h>
//#include <dbus/dbus-message.h>
//#include <dbus/dbus-pending-call.h>
//#include <dbus/dbus-protocol.h>
//#include <dbus/dbus-server.h>
//#include <dbus/dbus-shared.h>
//#include <dbus/dbus-threads.h>
//#include <dbus/dbus-types.h>
#include <dbus/dbus.h>

#include <devices/ahi.h>
#include <devices/audio.h>
#include <devices/bluetoothhci.h>
#include <devices/bootblock.h>
#include <devices/cd.h>
#include <devices/clipboard.h>
#include <devices/console.h>
#include <devices/conunit.h>
#include <devices/gameport.h>
#include <devices/hardblocks.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/irda.h>
#include <devices/keyboard.h>
#include <devices/keymap.h>
#include <devices/narrator.h>
#include <devices/newstyle.h>
#include <devices/parallel.h>
#include <devices/printer.h>
#include <devices/prtbase.h>
#include <devices/prtgfx.h>
#include <devices/rawkeycodes.h>
#include <devices/sana2.h>
#include <devices/sana2specialstats.h>
#include <devices/scsidisk.h>
#include <devices/serial.h>
#include <devices/timer.h>
#include <devices/trackdisk.h>
#include <devices/usb_audio.h>
#include <devices/usb_cdc.h>
#include <devices/usb_dfu.h>
#include <devices/usb_hid.h>
#include <devices/usb_hub.h>
#include <devices/usb_massstorage.h>
#include <devices/usb_printer.h>
#include <devices/usb_video.h>
#include <devices/usb.h>
#include <devices/usbhardware.h>

#include <dirent.h>

#include <diskfont/diskfont.h>
#include <diskfont/diskfonttag.h>
#include <diskfont/glyph.h>
#include <diskfont/oterrors.h>

#include <dos/bptr.h>
#include <dos/datetime.h>
#include <dos/dos.h>
#include <dos/dosasl.h>
#include <dos/dosextaros.h>
#include <dos/dosextens.h>
#include <dos/doshunks.h>
#include <dos/dostags.h>
#include <dos/elf.h>
#include <dos/exall.h>
#include <dos/filehandler.h>
#include <dos/filesystem.h>
#include <dos/notify.h>
#include <dos/rdargs.h>
#include <dos/record.h>
#include <dos/stdio.h>
#include <dos/var.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#include <endian.h>
#include <err.h>
#include <errno.h>

#include <exec/alerts.h>
#include <exec/avl.h>
#include <exec/devices.h>
#include <exec/errors.h>
#include <exec/exec.h>
#include <exec/execbase.h>
#include <exec/initializers.h>
#include <exec/interrupts.h>
#include <exec/io.h>
#include <exec/libraries.h>
#include <exec/lists.h>
#include <exec/memheaderext.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/ports.h>
#include <exec/rawfmt.h>
#include <exec/resident.h>
#include <exec/semaphores.h>
#include <exec/tasks.h>
#include <exec/types.h>

#include <fcntl.h>
#include <fenv.h>
#include <fpmath.h>

#include <freetype/config/ftconfig.h>
#include <freetype/config/ftheader.h>
//#include <freetype/config/ftmodule.h>
#include <freetype/config/ftoption.h>
#include <freetype/config/ftstdlib.h>
#include <freetype/freetype.h>
#include <freetype/ftbbox.h>
#include <freetype/ftbdf.h>
#include <freetype/ftbitmap.h>
#include <freetype/ftcache.h>
#include <freetype/ftchapters.h>
#include <freetype/ftcid.h>
#include <freetype/fterrdef.h>
#include <freetype/fterrors.h>
#include <freetype/ftgasp.h>
#include <freetype/ftglyph.h>
#include <freetype/ftgxval.h>
#include <freetype/ftgzip.h>
#include <freetype/ftimage.h>
#include <freetype/ftincrem.h>
#include <freetype/ftlcdfil.h>
#include <freetype/ftlist.h>
#include <freetype/ftlzw.h>
#include <freetype/ftmac.h>
#include <freetype/ftmm.h>
#include <freetype/ftmodapi.h>
#include <freetype/ftmoderr.h>
#include <freetype/ftotval.h>
#include <freetype/ftoutln.h>
#include <freetype/ftpfr.h>
#include <freetype/ftrender.h>
#include <freetype/ftsizes.h>
#include <freetype/ftsnames.h>
#include <freetype/ftstroke.h>
#include <freetype/ftsynth.h>
#include <freetype/ftsystem.h>
#include <freetype/fttrigon.h>
#include <freetype/fttypes.h>
#include <freetype/ftwinfnt.h>
#include <freetype/ftxf86.h>
//#include <freetype/internal/autohint.h>
//#include <freetype/internal/ftcalc.h>
//#include <freetype/internal/ftdebug.h>
//#include <freetype/internal/ftdriver.h>
//#include <freetype/internal/ftgloadr.h>
//#include <freetype/internal/ftmemory.h>
//#include <freetype/internal/ftobjs.h>
//#include <freetype/internal/ftrfork.h>
//#include <freetype/internal/ftserv.h>
//#include <freetype/internal/ftstream.h>
//#include <freetype/internal/fttrace.h>
//#include <freetype/internal/ftvalid.h>
//#include <freetype/internal/internal.h>
//#include <freetype/internal/pcftypes.h>
//#include <freetype/internal/psaux.h>
//#include <freetype/internal/pshints.h>
//#include <freetype/internal/sfnt.h>
//#include <freetype/internal/t1types.h>
//#include <freetype/internal/tttypes.h>
#include <freetype/t1tables.h>
#include <freetype/ttnameid.h>
#include <freetype/tttables.h>
#include <freetype/tttags.h>
#include <freetype/ttunpat.h>
#include <ft2build.h>

#include <gadgets/aroscheckbox.h>
#include <gadgets/aroscycle.h>
#include <gadgets/aroslist.h>
#include <gadgets/aroslistview.h>
#include <gadgets/arosmx.h>
#include <gadgets/arospalette.h>
#include <gadgets/colorwheel.h>
#include <gadgets/gradientslider.h>
#include <gadgets/tapedeck.h>

//#include <gallium/cso_cache/cso_cache.h>
//#include <gallium/cso_cache/cso_context.h>
//#include <gallium/cso_cache/cso_hash.h>
//#include <gallium/draw/draw_cliptest_tmp.h>
//#include <gallium/draw/draw_context.h>
//#include <gallium/draw/draw_decompose_tmp.h>
//#include <gallium/draw/draw_fs.h>
//#include <gallium/draw/draw_gs_tmp.h>
//#include <gallium/draw/draw_gs.h>
//#include <gallium/draw/draw_llvm.h>
//#include <gallium/draw/draw_pipe.h>
//#include <gallium/draw/draw_private.h>
//#include <gallium/draw/draw_pt_decompose.h>
//#include <gallium/draw/draw_pt_vsplit_tmp.h>
//#include <gallium/draw/draw_pt.h>
//#include <gallium/draw/draw_so_emit_tmp.h>
//#include <gallium/draw/draw_split_tmp.h>
//#include <gallium/draw/draw_vbuf.h>
//#include <gallium/draw/draw_vertex.h>
//#include <gallium/draw/draw_vs_aos.h>
//#include <gallium/draw/draw_vs.h>
#include <gallium/gallium.h>
//#include <gallium/indices/u_indices_priv.h>
//#include <gallium/indices/u_indices.h>
//#include <gallium/os/os_memory_aligned.h>
//#include <gallium/os/os_memory_debug.h>
//#include <gallium/os/os_memory_stdc.h>
//#include <gallium/os/os_memory_win32k.h>
//#include <gallium/os/os_memory.h>
//#include <gallium/os/os_misc.h>
//#include <gallium/os/os_stream.h>
//#include <gallium/os/os_thread.h>
//#include <gallium/os/os_time.h>
//#include <gallium/pipe/p_aros_version.h>
//#include <gallium/pipe/p_compiler.h>
//#include <gallium/pipe/p_config.h>
//#include <gallium/pipe/p_context.h>
//#include <gallium/pipe/p_defines.h>
//#include <gallium/pipe/p_format.h>
//#include <gallium/pipe/p_screen.h>
//#include <gallium/pipe/p_shader_tokens.h>
//#include <gallium/pipe/p_state.h>
//#include <gallium/pipebuffer/pb_buffer_fenced.h>
//#include <gallium/pipebuffer/pb_buffer.h>
//#include <gallium/pipebuffer/pb_bufmgr.h>
//#include <gallium/pipebuffer/pb_validate.h>
//#include <gallium/rbug/rbug_connection.h>
//#include <gallium/rbug/rbug_context.h>
//#include <gallium/rbug/rbug_core.h>
//#include <gallium/rbug/rbug_internal.h>
//#include <gallium/rbug/rbug_proto.h>
//#include <gallium/rbug/rbug_shader.h>
//#include <gallium/rbug/rbug_texture.h>
//#include <gallium/rbug/rbug.h>
//#include <gallium/rtasm/rtasm_cpu.h>
//#include <gallium/rtasm/rtasm_execmem.h>
//#include <gallium/rtasm/rtasm_ppc_spe.h>
//#include <gallium/rtasm/rtasm_ppc.h>
//#include <gallium/rtasm/rtasm_x86sse.h>
//#include <gallium/tgsi/tgsi_build.h>
//#include <gallium/tgsi/tgsi_dump.h>
//#include <gallium/tgsi/tgsi_exec.h>
//#include <gallium/tgsi/tgsi_info.h>
//#include <gallium/tgsi/tgsi_iterate.h>
//#include <gallium/tgsi/tgsi_opcode_tmp.h>
//#include <gallium/tgsi/tgsi_parse.h>
//#include <gallium/tgsi/tgsi_ppc.h>
//#include <gallium/tgsi/tgsi_sanity.h>
//#include <gallium/tgsi/tgsi_scan.h>
//#include <gallium/tgsi/tgsi_sse2.h>
//#include <gallium/tgsi/tgsi_text.h>
//#include <gallium/tgsi/tgsi_transform.h>
//#include <gallium/tgsi/tgsi_ureg.h>
//#include <gallium/tgsi/tgsi_util.h>
//#include <gallium/translate/translate_cache.h>
//#include <gallium/translate/translate.h>
//#include <gallium/util/u_atomic.h>
//#include <gallium/util/u_bitmask.h>
//#include <gallium/util/u_blit.h>
//#include <gallium/util/u_blitter.h>
//#include <gallium/util/u_box.h>
//#include <gallium/util/u_cache.h>
//#include <gallium/util/u_caps.h>
//#include <gallium/util/u_clear.h>
//#include <gallium/util/u_cpu_detect.h>
//#include <gallium/util/u_debug_describe.h>
//#include <gallium/util/u_debug_refcnt.h>
//#include <gallium/util/u_debug_stack.h>
//#include <gallium/util/u_debug_symbol.h>
//#include <gallium/util/u_debug.h>
//#include <gallium/util/u_dirty_flags.h>
//#include <gallium/util/u_dirty_surfaces.h>
//#include <gallium/util/u_dl.h>
//#include <gallium/util/u_double_list.h>
//#include <gallium/util/u_draw_quad.h>
//#include <gallium/util/u_draw.h>
//#include <gallium/util/u_dump.h>
//#include <gallium/util/u_dynarray.h>
//#include <gallium/util/u_fifo.h>
//#include <gallium/util/u_format_other.h>
//#include <gallium/util/u_format_s3tc.h>
//#include <gallium/util/u_format_srgb.h>
//#include <gallium/util/u_format_tests.h>
//#include <gallium/util/u_format_yuv.h>
//#include <gallium/util/u_format_zs.h>
//#include <gallium/util/u_format.h>
//#include <gallium/util/u_framebuffer.h>
//#include <gallium/util/u_gen_mipmap.h>
//#include <gallium/util/u_half.h>
//#include <gallium/util/u_handle_table.h>
//#include <gallium/util/u_hash_table.h>
//#include <gallium/util/u_hash.h>
//#include <gallium/util/u_index_modify.h>
//#include <gallium/util/u_init.h>
//#include <gallium/util/u_inlines.h>
//#include <gallium/util/u_keymap.h>
//#include <gallium/util/u_linear.h>
//#include <gallium/util/u_linkage.h>
//#include <gallium/util/u_math.h>
//#include <gallium/util/u_memory.h>
//#include <gallium/util/u_mm.h>
//#include <gallium/util/u_network.h>
//#include <gallium/util/u_pack_color.h>
//#include <gallium/util/u_pointer.h>
//#include <gallium/util/u_prim.h>
//#include <gallium/util/u_rect.h>
//#include <gallium/util/u_ringbuffer.h>
//#include <gallium/util/u_sampler.h>
//#include <gallium/util/u_simple_list.h>
//#include <gallium/util/u_simple_screen.h>
//#include <gallium/util/u_simple_shaders.h>
//#include <gallium/util/u_slab.h>
//#include <gallium/util/u_split_prim.h>
//#include <gallium/util/u_sse.h>
//#include <gallium/util/u_staging.h>
//#include <gallium/util/u_string.h>
//#include <gallium/util/u_surface.h>
//#include <gallium/util/u_surfaces.h>
//#include <gallium/util/u_texture.h>
//#include <gallium/util/u_tile.h>
//#include <gallium/util/u_time.h>
//#include <gallium/util/u_transfer.h>
//#include <gallium/util/u_upload_mgr.h>

#include <getopt.h>

#include <GL/arosmesa.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>

#include <graphics/clip.h>
#include <graphics/collide.h>
#include <graphics/copper.h>
#include <graphics/displayinfo.h>
#include <graphics/driver.h>
#include <graphics/gels.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#include <graphics/gfxnodes.h>
#include <graphics/layers.h>
#include <graphics/layersext.h>
#include <graphics/modeid.h>
#include <graphics/monitor.h>
#include <graphics/rastport.h>
#include <graphics/regions.h>
#include <graphics/rpattr.h>
#include <graphics/scale.h>
#include <graphics/sprite.h>
#include <graphics/text.h>
#include <graphics/videocontrol.h>
#include <graphics/view.h>

#include <grp.h>

#include <hardware/ahci.h>
#include <hardware/blit.h>
#include <hardware/cia.h>
#include <hardware/cpu/cpu_i386.h>
#include <hardware/cpu/cpu_m68k.h>
#include <hardware/cpu/cpu_mpspec.h>
#include <hardware/cpu/cpu.h>
#include <hardware/cpu/memory.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <hardware/pic/pic.h>
#include <hardware/uart.h>
#include <hardware/vbe.h>

#include <hidd/agp.h>
#include <hidd/config.h>
#include <hidd/gallium.h>
#include <hidd/graphics_inline.h>
#include <hidd/graphics.h>
#include <hidd/hidd.h>
#include <hidd/i2c.h>
#include <hidd/irq.h>
#include <hidd/keyboard.h>
#include <hidd/mouse.h>
#include <hidd/parallel.h>
#include <hidd/pci.h>
#include <hidd/serial.h>
#include <hidd/unixio_inline.h>
#include <hidd/unixio.h>

#include <inetd.h>
#include <inttypes.h>

#include <intuition/cghooks.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/extensions.h>
#include <intuition/gadgetclass.h>
#include <intuition/icclass.h>
#include <intuition/imageclass.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/iobsolete.h>
#include <intuition/iprefs.h>
#include <intuition/menudecorclass.h>
#include <intuition/monitorclass.h>
#include <intuition/pointerclass.h>
#include <intuition/preferences.h>
#include <intuition/scrdecorclass.h>
#include <intuition/screens.h>
#include <intuition/sghooks.h>
#include <intuition/windecorclass.h>

#include <ioerr2errno.h>
#include <irda/irlap.h>
#include <iso646.h>
#include <jconfig.h>
#include <jerror.h>
#include <jinclude.h>
#include <jmorecfg.h>
#include <jpeglib.h>
#include <jversion.h>
#include <KHR/khrplatform.h>
#include <libcore/compiler.h>
#include <libgen.h>

#include <libraries/ahi_sub.h>
#include <libraries/amigaguide.h>
#include <libraries/arp.h>
#include <libraries/asl.h>
#include <libraries/bsdsocket.h>
#include <libraries/codesets.h>
#include <libraries/commodities.h>
#include <libraries/configregs.h>
#include <libraries/configvars.h>
#include <libraries/coolimages.h>
#include <libraries/cybergraphics.h>
#include <libraries/debug.h>
#include <libraries/desktop.h>
#include <libraries/diskfont.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <libraries/expansion.h>
#include <libraries/expansionbase.h>
#include <libraries/filehandler.h>
#include <libraries/gadtools.h>
#include <libraries/identify.h>
#include <libraries/iffparse.h>
#include <libraries/kms.h>
#include <libraries/locale.h>
#include <libraries/lowlevel_ext.h>
#include <libraries/lowlevel.h>
#include <libraries/mathffp.h>
#include <libraries/mathieeedp.h>
#include <libraries/mathieeesp.h>
#include <libraries/miami.h>
#include <libraries/miamipanel.h>
#include <libraries/mui.h>
#include <libraries/muiscreen.h>
#include <libraries/nonvolatile.h>
#include <libraries/openurl.h>
#include <libraries/partition.h>
#include <libraries/pm.h>
#include <libraries/poseidon.h>
#include <libraries/prometheus.h>
#include <libraries/realtime.h>
#include <libraries/reqtools.h>
#include <libraries/security.h>
#include <libraries/thread.h>
#include <libraries/usbclass.h>
#include <libraries/usergroup.h>
#include <libraries/uuid.h>

#include <limits.h>
#include <linklibs/coolimages.h>
#include <locale.h>

// There is an incompatibility between ANSI math and AROS math.
// Disabled for now to get rid of a lot of compiler errors.
//#include <math.h>

#include <memory.h>

#include <midi/camd.h>
#include <midi/camddevices.h>
#include <midi/mididefs.h>

#include <mui/BetterString_mcc.h>
#include <mui/Busy_mcc.h>
#include <mui/HotkeyString_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <mui/TextEditor_mcp.h>

#include <net/if_arp.h>
#include <net/if_dl.h>
#include <net/if_llc.h>
#include <net/if_slvar.h>
#include <net/if_types.h>
#include <net/if.h>
#include <net/radix.h>
#include <net/route.h>
#include <net/sana2errno.h>
#include <netdb.h>
#include <netinet/icmp_var.h>
#include <netinet/in_pcb.h>
#include <netinet/in_systm.h>
#include <netinet/in_var.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <netinet/ip.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp.h>
#include <netinet/udp_var.h>
#include <netinet/udp.h>

#include <oop/ifmeta.h>
#include <oop/oop.h>
#include <oop/static_mid.h>

#include <png.h>
#include <pngconf.h>
#include <pnglibconf.h>

#include <prefs/font.h>
#include <prefs/icontrol.h>
#include <prefs/input.h>
#include <prefs/locale.h>
#include <prefs/overscan.h>
#include <prefs/palette.h>
#include <prefs/pointer.h>
#include <prefs/prefhdr.h>
#include <prefs/printergfx.h>
#include <prefs/printerps.h>
#include <prefs/printertxt.h>
#include <prefs/screenmode.h>
#include <prefs/serial.h>
#include <prefs/sound.h>
#include <prefs/trackdisk.h>
#include <prefs/wanderer.h>
#include <prefs/wbpattern.h>
#include <prefs/workbench.h>

#include <process.h>

#include <proto/8svx.h>
#include <proto/Aboutmui.h>
#include <proto/AboutWindow.h>
#include <proto/ahi_sub.h>
#include <proto/ahi.h>
#include <proto/alib.h>
#include <proto/amigaguide.h>
#include <proto/aros.h>
#include <proto/aroscheckbox.h>
#include <proto/aroscycle.h>
#include <proto/aroslist.h>
#include <proto/aroslistview.h>
#include <proto/arosmutualexclude.h>
#include <proto/arospalette.h>
#include <proto/arossupport.h>
#include <proto/ascii.h>
#include <proto/asl.h>
#include <proto/Balance.h>
#include <proto/battclock.h>
#include <proto/battmem.h>
#include <proto/binary.h>
#include <proto/bmp.h>
#include <proto/Boopsi.h>
#include <proto/bootloader.h>
#include <proto/bsdsocket.h>
#include <proto/bullet.h>
#include <proto/Busy.h>
#include <proto/Calendar.h>
#include <proto/camd.h>
#include <proto/cardres.h>
#include <proto/cia.h>
#include <proto/Clock.h>
#include <proto/codesets.h>
#include <proto/Coloradjust.h>
#include <proto/Colorfield.h>
#include <proto/colorwheel.h>
#include <proto/commodities.h>
#include <proto/console.h>
#include <proto/coolimages.h>
#include <proto/Crawling.h>
#include <proto/cybergraphics.h>
#include <proto/datatypes.h>
#include <proto/debug.h>
#include <proto/Dirlist.h>
#include <proto/disk.h>
#include <proto/diskfont.h>
#include <proto/dos.h>
#include <proto/dtclass.h>
#include <proto/Dtpic.h>
#include <proto/egl.h>
#include <proto/exec_sysbase.h>
#include <proto/exec.h>
#include <proto/expansion.h>
#include <proto/Floattext.h>
#include <proto/freetype2.h>
#include <proto/gadtools.h>
#include <proto/gallium.h>
#include <proto/gameport.h>
#include <proto/gif.h>
#include <proto/glu.h>
#include <proto/gradientslider.h>
#include <proto/graphics.h>
#include <proto/hostlib.h>
#include <proto/html.h>
#include <proto/icon.h>
#include <proto/IconDrawerList.h>
#include <proto/IconImage.h>
#include <proto/IconList.h>
#include <proto/IconListview.h>
#include <proto/IconVolumeList.h>
#include <proto/identify.h>
#include <proto/iffparse.h>
#include <proto/ilbm.h>
#include <proto/input.h>
#include <proto/intuition.h>
#include <proto/jpeg.h>
#include <proto/kernel.h>
#include <proto/keymap.h>
#include <proto/kms.h>
#include <proto/Knob.h>
#include <proto/layers.h>
#include <proto/Levelmeter.h>
#include <proto/locale.h>
#include <proto/lowlevel.h>
#include <proto/mathffp.h>
#include <proto/mathieeedoubbas.h>
#include <proto/mathieeedoubtrans.h>
#include <proto/mathieeesingbas.h>
#include <proto/mathieeesingtrans.h>
#include <proto/mathtrans.h>
#include <proto/mesa.h>
#include <proto/miami.h>
#include <proto/miamipanel.h>
#include <proto/misc.h>
#include <proto/muimaster.h>
#include <proto/muiscreen.h>
#include <proto/nonvolatile.h>
#include <proto/nvdisk.h>
#include <proto/oop.h>
#include <proto/openurl.h>
#include <proto/oss.h>
#include <proto/Palette.h>
#include <proto/partition.h>
#include <proto/picture.h>
#include <proto/png.h>
#include <proto/pnm.h>
#include <proto/Popasl.h>
#include <proto/Popframe.h>
#include <proto/Popimage.h>
#include <proto/Poplist.h>
#include <proto/Poppen.h>
#include <proto/Popscreen.h>
#include <proto/popupmenu.h>
#include <proto/potgo.h>
#include <proto/PrefsEditor.h>
#include <proto/PrefsWindow.h>
#include <proto/processor.h>
#include <proto/prometheus.h>
#include <proto/Radio.h>
#include <proto/ramdrive.h>
#include <proto/realtime.h>
#include <proto/reqtools.h>
#include <proto/rexxsupport.h>
#include <proto/rexxsyslib.h>
#include <proto/Scrollgroup.h>
#include <proto/Settingsgroup.h>
#include <proto/socket.h>
#include <proto/sound.h>
#include <proto/SystemPrefsWindow.h>
#include <proto/text.h>
#include <proto/timer.h>
#include <proto/translator.h>
#include <proto/utility.h>
#include <proto/uuid.h>
#include <proto/vega.h>
#include <proto/version.h>
#include <proto/Virtgroup.h>
#include <proto/Volumelist.h>
#include <proto/wb.h>
#include <proto/workbench.h>

#include <pwd.h>
#include <regex.h>

#include <resources/battclock.h>
#include <resources/card.h>
#include <resources/cia.h>
#include <resources/disk.h>
#include <resources/filesysres.h>
#include <resources/isapnp.h>
#include <resources/misc.h>
#include <resources/potgo.h>
#include <resources/processor.h>

#include <rexx/errors.h>
#include <rexx/rexxcall-m68k.h>
#include <rexx/rexxcall.h>
#include <rexx/rxslib.h>
#include <rexx/storage.h>

#include <scsi/commands.h>
#include <scsi/values.h>

#include <sdgstd.h>

#include <SDI/SDI_compiler.h>
#include <SDI/SDI_hook.h>
#include <SDI/SDI_interrupt.h>
#include <SDI/SDI_lib.h>
#include <SDI/SDI_misc.h>
#include <SDI/SDI_stdarg.h>

#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <sys/_types.h>
#include <sys/arosc.h>
#include <sys/cdefs.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/fs_types.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/net_types.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/syslog.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/times.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/unistd.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <syslog.h>

#include <termios.h>
#include <time.h>

//disabled because it redefines some AROS types
//#include <toollib/callback.h>
//#include <toollib/error.h>
//#include <toollib/filesup.h>
//#include <toollib/hash.h>
//#include <toollib/lineparser.h>
//#include <toollib/mystream.h>
//#include <toollib/stdiocb.h>
//#include <toollib/stringcb.h>
//#include <toollib/toollib.h>
//#include <toollib/vstring.h>

#include <ucontext.h>
#include <unistd.h>

#include <usb/hid.h>
#include <usb/storage.h>
#include <usb/usb_core.h>
#include <usb/usb.h>

#include <utility/date.h>
#include <utility/hooks.h>
#include <utility/name.h>
#include <utility/pack.h>
#include <utility/tagitem.h>
#include <utility/utility.h>

#include <utime.h>
#include <values.h>

#include <VG/openvg.h>
#include <VG/vgext.h>
#include <VG/vgplatform.h>
#include <VG/vgu.h>

#include <wchar.h>
#include <wctype.h>

#include <workbench/handler.h>
#include <workbench/icon.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>

#include <zconf.h>
#include <zlib.h>

#include <zune/aboutwindow.h>
#include <zune/calendar.h>
#include <zune/clock.h>
#include <zune/customclasses.h>
#include <zune/iconimage.h>
#include <zune/loginwindow.h>
#include <zune/prefseditor.h>
#include <zune/prefswindow.h>
#include <zune/systemprefswindow.h>

#include <zutil.h>

int main(void)
{
    return 0;
}
