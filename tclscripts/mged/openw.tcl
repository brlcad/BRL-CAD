# Author -
#	Bob Parker
#
# Source -
#	The U. S. Army Research Laboratory
#	Aberdeen Proving Ground, Maryland  21005
#
# Distribution Notice -
#	Re-distribution of this software is restricted, as described in
#       your "Statement of Terms and Conditions for the Release of
#       The BRL-CAD Package" agreement.
#
# Copyright Notice -
#       This software is Copyright (C) 1998 by the United States Army
#       in all countries except the USA.  All rights reserved.
#
# Description -
#	MGED's default Tcl/Tk interface.
#
# $Revision
#

check_externs "_mged_attach _mged_tie _mged_view_ring"

if [info exists env(MGED_HTML_DIR)] {
        set mged_html_dir $env(MGED_HTML_DIR)
} else {
        set mged_html_dir [lindex $auto_path 0]/../html/mged
}

if ![info exists mged_color_scheme] {
    color_scheme_init
}

if ![info exists mged_players] {
    set mged_players {}
}

if ![info exists mged_collaborators] {
    set mged_collaborators {}
}

if ![info exists mged_default(ggeom)] {
    set mged_default(ggeom) -0+0
}

if ![info exists mged_default(geom)] {
    set mged_default(geom) +8+32
}

if ![info exists mged_default(id)] {
    set mged_default(id) "id"
}

if ![info exists mged_default(pane)] {
    set mged_default(pane) "ur"
}

if ![info exists mged_default(multi_view)] {
    set mged_default(multi_view) 0
}

if ![info exists mged_default(config)] {
    set mged_default(config) g
}

if ![info exists mged_default(display)] {
    if [info exists env(DISPLAY)] {
	set mged_default(display) $env(DISPLAY)
    } else {
	set mged_default(display) :0
    }
}

if ![info exists mged_default(gdisplay)] {
    if [info exists env(DISPLAY)] {
	set mged_default(gdisplay) $env(DISPLAY)
    } else {
	set mged_default(gdisplay) :0
    }
}

if ![info exists mged_default(dt)] {
    set mged_default(dt) [dm_bestXType $mged_default(gdisplay)]
}

if ![info exists mged_default(comb)] {
    set mged_default(comb) 0
}

if ![info exists mged_default(edit_style)] {
    set mged_default(edit_style) emacs
}

if ![info exists mged_default(num_lines)] {
    set mged_default(num_lines) 10
}

if ![info exists mged_default(tearoff_menus)] {
    set mged_default(tearoff_menus) 0
}

if ![info exists mged_gui(mged,screen)] {
    set mged_gui(mged,screen) $mged_default(display)
}

if ![info exists tk_version] {
    loadtk
}

##
# Set the class bindings for use with help. This requires the
# widget to register its data using hoc_register_data. Also, for now,
# the Menu class binding is being handled in tclscripts/menu_override.tcl.
#
bind Entry <ButtonPress-3><ButtonRelease-3> "hoc_callback %W %X %Y"
bind Label <ButtonPress-3><ButtonRelease-3> "hoc_callback %W %X %Y"
bind Text <ButtonPress-3><ButtonRelease-3> "hoc_callback %W %X %Y"
bind Button <ButtonPress-3><ButtonRelease-3> "hoc_callback %W %X %Y"
bind Checkbutton <ButtonPress-3><ButtonRelease-3> "hoc_callback %W %X %Y"
bind Menubutton <ButtonPress-3><ButtonRelease-3> "hoc_callback %W %X %Y"
bind Radiobutton <ButtonPress-3><ButtonRelease-3> "hoc_callback %W %X %Y"

proc gui { args } {
    global mged_gui
    global mged_default
    global mged_html_dir
    global mged_players
    global mged_collaborators
    global mged_display
    global env
    global view_ring
    global vi_state
    global mged_color_scheme

    if {$mged_default(dt) == ""} {
	set mged_default(dt) [dm_bestXType $mged_default(gdisplay)]
    }

# set defaults
    set save_id [cmd_win get]
    set comb $mged_default(comb)
    set join_c 0
    set dtype $mged_default(dt)
    set id ""
    set scw 0
    set sgw 0
    set i 0

    set screen $mged_default(display)
    set gscreen $mged_default(gdisplay)

    if {$mged_default(config) == "b"} {
	set scw 1
	set sgw 1
    } elseif {$mged_default(config) == "c"} {
	set scw 1
	set sgw 0
    } elseif {$mged_default(config) == "g"} {
	set scw 0
	set sgw 1
    }

#==============================================================================
# PHASE 0: Process options
#==============================================================================
    set argc [llength $args]
    set dtype_set 0
    set screen_set 0
    set gscreen_set 0

for {set i 0} {$i < $argc} {incr i} {
    set arg [lindex $args $i]
    if {$arg == "-config"} {
	incr i

	if {$i >= $argc} {
	    return [help gui]
	}

	set arg [lindex $args $i]
	if {$arg == "b"} {
# show both command and graphics windows
	    set scw 1
	    set sgw 1
	} elseif { $arg == "c"} {
	    set scw 1
	    set sgw 0
	} elseif {$arg == "g"} {
# show display manager window
	    set scw 0
	    set sgw 1
	} else {
	    return [help gui]
	}
    } elseif {$arg == "-d" || $arg == "-display"} {
# display string for everything
	incr i

	if {$i >= $argc} {
	    return [help gui]
	}

	set screen [lindex $args $i]
	set screen_set 1
	if {!$gscreen_set} {
	    set gscreen $screen

	    if {!$dtype_set} {
		set dtype [dm_bestXType $gscreen]
	    }
	}
    } elseif {$arg == "-gd" || $arg == "-gdisplay"} {
# display string for graphics window
	incr i

	if {$i >= $argc} {
	    return [help gui]
	}

	set gscreen [lindex $args $i]
	set gscreen_set 1
	if {!$screen_set} {
	    set screen $gscreen
	}

	if {!$dtype_set} {
	    set dtype [dm_bestXType $gscreen]
	}
    } elseif {$arg == "-dt"} {
	incr i

	if {$i >= $argc} {
	    return [help gui]
	}

	set dtype [lindex $args $i]
	set dtype_set 1
    } elseif {$arg == "-id"} {
	incr i

	if {$i >= $argc} {
	    return [help gui]
	}

	set id [lindex $args $i]
    } elseif {$arg == "-j" || $arg == "-join"} { 
	set join_c 1
    } elseif {$arg == "-h" || $arg == "-help"} {
	return [help gui]
    } elseif {$arg == "-s" || $arg == "-sep"} {
	set comb 0
    } elseif {$arg == "-c" || $arg == "-comb"} {
	set comb 1
    } else {
	return [help gui]
    }
}

if {$comb} {
    set gscreen $screen
}

if {$id == "mged"} {
    return "gui: not allowed to use \"mged\" as id"
}

if {$id == ""} {
    for {set i 0} {1} {incr i} {
	set id [subst $mged_default(id)]_$i
	if {[lsearch -exact $mged_players $id] == -1} {
	    break;
	}
    }
}

if {$scw == 0 && $sgw == 0} {
    set sgw 1
}

set mged_gui($id,comb) $comb
set mged_gui($id,show_cmd) $scw
set mged_gui($id,show_dm) $sgw
set mged_gui($id,show_status) 1
set mged_gui($id,apply_to) 0
set mged_gui($id,edit_info_pos) "+0+0"
set mged_gui($id,num_lines) $mged_default(num_lines)
set mged_gui($id,multi_view) $mged_default(multi_view)
set mged_gui($id,dm_loc) $mged_default(pane)
set mged_gui($id,dtype) $dtype

if ![dm_validXType $gscreen $dtype] {
    return "gui: $gscreen does not support $dtype"
}

if { [info exists tk_strictMotif] == 0 } {
    loadtk $screen
}

#==============================================================================
# PHASE 1: Creation of main window
#==============================================================================
if [ winfo exists .$id ] {
    return "A session with the id \"$id\" already exists!"
}
	
toplevel .$id -screen $screen -menu .$id.menubar

lappend mged_players $id
set mged_gui($id,screen) $screen

#==============================================================================
# Create display manager window and menu
#==============================================================================
if {$comb} {
    set mged_gui($id,top) .$id
    set mged_gui($id,dmc) .$id.dmf

    frame $mged_gui($id,dmc) -relief sunken -borderwidth 2

    if [catch { openmv $id $mged_gui($id,top) $mged_gui($id,dmc) $screen $dtype } result] {
	gui_destroy $id
	return $result
    }
} else {
    set mged_gui($id,top) .top$id
    set mged_gui($id,dmc) $mged_gui($id,top)

    toplevel $mged_gui($id,dmc) -screen $gscreen -relief sunken -borderwidth 2

    if [catch { openmv $id $mged_gui($id,top) $mged_gui($id,dmc) $gscreen $dtype } result] {
	gui_destroy $id
	return $result
    }
}

set mged_gui($id,active_dm) $mged_gui($id,top).$mged_default(pane)
set mged_gui($id,apply_list) $mged_gui($id,active_dm)

#==============================================================================
# PHASE 2: Construction of menu bar
#==============================================================================

menu .$id.menubar -tearoff $mged_default(tearoff_menus)
.$id.menubar add cascade -label "File" -underline 0 -menu .$id.menubar.file
.$id.menubar add cascade -label "Edit" -underline 0 -menu .$id.menubar.edit
.$id.menubar add cascade -label "Create" -underline 0 -menu .$id.menubar.create
.$id.menubar add cascade -label "View" -underline 0 -menu .$id.menubar.view
.$id.menubar add cascade -label "ViewRing" -underline 4 -menu .$id.menubar.viewring
.$id.menubar add cascade -label "Settings" -underline 0 -menu .$id.menubar.settings
.$id.menubar add cascade -label "Modes" -underline 0 -menu .$id.menubar.modes
.$id.menubar add cascade -label "Misc" -underline 1 -menu .$id.menubar.misc
.$id.menubar add cascade -label "Tools" -underline 0 -menu .$id.menubar.tools
.$id.menubar add cascade -label "Help" -underline 0 -menu .$id.menubar.help

menu .$id.menubar.file -title "File" -tearoff $mged_default(tearoff_menus)
.$id.menubar.file add command -label "New..." -underline 0 -command "do_New $id"
hoc_register_menu_data "File" "New..." "New Database"\
	{ { summary "Open a new database. Note - the database\nmust not already exist." }
          { see_also opendb} }
.$id.menubar.file add command -label "Open..." -underline 0 -command "do_Open $id"
hoc_register_menu_data "File" "Open..." "Open Database"\
	{ { summary "Open an existing database." }
          { see_also opendb } }
.$id.menubar.file add command -label "Insert..." -underline 0 -command "do_Concat $id"
hoc_register_menu_data "File" "Insert..." "Insert Database"\
	{ { summary "Insert another database into the current database." }
          { see_also dbconcat } }
.$id.menubar.file add command -label "Extract..." -underline 1 -command "init_extractTool $id"
hoc_register_menu_data "File" "Extract..." "Extract Objects"\
	{ { summary "Tool for extracting objects out of the current database." }
          { see_also keep } }
.$id.menubar.file add command -label "g2asc..." -underline 0 -command "init_g2asc $id"
hoc_register_menu_data "File" "g2asc..." "Convert to Ascii"\
	{ { summary "Convert the current database into an ascii file." } }
.$id.menubar.file add separator
.$id.menubar.file add command -label "Raytrace..." -underline 0 -command "init_Raytrace $id"
hoc_register_menu_data "File" "Raytrace..." "Raytrace View"\
	{ { summary "Tool for raytracing the current view." }
          { see_also rt } }
.$id.menubar.file add cascade -label "Save View As" -underline 0 -menu .$id.menubar.file.saveview
.$id.menubar.file add separator
.$id.menubar.file add cascade -label "Preferences" -underline 0 -menu .$id.menubar.file.pref
.$id.menubar.file add separator
.$id.menubar.file add command -label "Close" -underline 0 -command "gui_destroy $id"
hoc_register_menu_data "File" "Close" "Close Window"\
	{ { summary "Close this graphical user interface." }
          { see_also } }
.$id.menubar.file add command -label "Exit" -underline 0 -command _mged_quit
hoc_register_menu_data "File" "Exit" "Exit MGED"\
	{ { summary "Exit MGED." }
          { see_also "exit q quit" } }

menu .$id.menubar.file.saveview -title "Save View As" -tearoff $mged_default(tearoff_menus)
.$id.menubar.file.saveview add command -label "RT script..." -underline 0\
	-command "init_rtScriptTool $id"
hoc_register_menu_data "Save View As" "RT script..." "RT Script File"\
	{ { summary "Save the current view as an RT script file." }
          { see_also saveview } }
.$id.menubar.file.saveview add command -label "Plot..." -underline 1\
	-command "init_plotTool $id"
hoc_register_menu_data "Save View As" "Plot..." "Plot File"\
	{ { summary "Save the current view as a Plot file." }
          { see_also pl } }
.$id.menubar.file.saveview add command -label "PostScript..." -underline 0\
	-command "init_psTool $id"
hoc_register_menu_data "Save View As" "PostScript..." "PostScript File"\
	{ { summary "Save the current view as a PostScript file." }
          { see_also ps } }

menu .$id.menubar.file.pref -title "Preferences" -tearoff $mged_default(tearoff_menus)
.$id.menubar.file.pref add cascade -label "Units" -underline 0\
	-menu .$id.menubar.file.pref.units
.$id.menubar.file.pref add cascade -label "Command Line Edit" -underline 0\
	-menu .$id.menubar.file.pref.cle
.$id.menubar.file.pref add cascade -label "Special Characters" -underline 0\
	-menu .$id.menubar.file.pref.special_chars
.$id.menubar.file.pref add command -label "Color Schemes..." -underline 6\
	-command "color_scheme_build $id \"Color Schemes\" [list $mged_color_scheme(primary_map)]\
	\"Faceplate Colors\" [list $mged_color_scheme(secondary_map)]"
hoc_register_menu_data "Preferences" "Color Schemes..." "Color Schemes"\
	{ { summary "Tool for setting colors." }
          { see_also "rset" } }

menu .$id.menubar.file.pref.units -title "Units" -tearoff $mged_default(tearoff_menus)
.$id.menubar.file.pref.units add radiobutton -value um -variable mged_display(units)\
	-label "micrometers" -underline 4 -command "do_Units $id"
hoc_register_menu_data "Units" "micrometers" "Unit of measure - Micrometers"\
	{ { summary "Set the unit of measure to micrometers.\n1 micrometer = 1/1,000,000 meters" }
          { see_also "units" } }
.$id.menubar.file.pref.units add radiobutton -value mm -variable mged_display(units)\
	-label "millimeters" -underline 2 -command "do_Units $id"
hoc_register_menu_data "Units" "millimeters" "Unit of measure - Millimeters"\
	{ { summary "Set the unit of measure to millimeters.\n1 millimeter = 1/1000 meters" }
          { see_also "units" } }
.$id.menubar.file.pref.units add radiobutton -value cm -variable mged_display(units)\
	-label "centimeters" -underline 0 -command "do_Units $id"
hoc_register_menu_data "Units" "centimeters" "Unit of measure - Centimeters"\
	{ { summary "Set the unit of measure to centimeters.\n\t1 centimeter = 1/100 meters" }
          { see_also "units" } }
.$id.menubar.file.pref.units add radiobutton -value m -variable mged_display(units)\
	-label "meters" -underline 0 -command "do_Units $id"
hoc_register_menu_data "Units" "meters" "Unit of measure - Meters"\
	{ { summary "Set the unit of measure to meters." }
          { see_also "units" } }
.$id.menubar.file.pref.units add radiobutton -value km -variable mged_display(units)\
	-label "kilometers" -underline 0 -command "do_Units $id"
hoc_register_menu_data "Units" "kilometers" "Unit of measure - Kilometers"\
	{ { summary "Set the unit of measure to kilometers.\n 1 kilometer = 1000 meters" }
          { see_also "units" } }
.$id.menubar.file.pref.units add separator
.$id.menubar.file.pref.units add radiobutton -value in -variable mged_display(units)\
	-label "inches" -underline 0 -command "do_Units $id"
hoc_register_menu_data "Units" "inches" "Unit of measure - Inches"\
	{ { summary "Set the unit of measure to inches.\n 1 inch = 25.4 mm" }
          { see_also "units" } }
.$id.menubar.file.pref.units add radiobutton -value ft -variable mged_display(units)\
	-label "feet" -underline 0 -command "do_Units $id"
hoc_register_menu_data "Units" "feet" "Unit of measure - Feet"\
	{ { summary "Set the unit of measure to feet.\n 1 foot = 12 inches" }
          { see_also "units" } }
.$id.menubar.file.pref.units add radiobutton -value yd -variable mged_display(units)\
	-label "yards" -underline 0 -command "do_Units $id"
hoc_register_menu_data "Units" "yards" "Unit of measure - Yards"\
	{ { summary "Set the unit of measure to yards.\n 1 yard = 36 inches" }
          { see_also "" } }
.$id.menubar.file.pref.units add radiobutton -value mi -variable mged_display(units)\
	-label "miles" -underline 3 -command "do_Units $id"
hoc_register_menu_data "Units" "miles" "Unit of measure - Miles"\
	{ { summary "Set the unit of measure to miles.\n 1 mile = 5280 feet" }
          { see_also "units" } }

menu .$id.menubar.file.pref.cle -title "Command Line Edit" -tearoff $mged_default(tearoff_menus)
.$id.menubar.file.pref.cle add radiobutton -value emacs -variable mged_gui($id,edit_style)\
	-label "emacs" -underline 0 -command "set_text_key_bindings $id"
hoc_register_menu_data "Command Line Edit" "emacs" "Command Line Edit - Emacs"\
	{ { synopsis "Set the command line edit mode to emacs." }
          { description "
\tBackSpace\tbackward delete char
\tDelete\t\tbackward delete char
\tLeft\t\tbackward char
\tRight\t\tforward char
\tUp\t\tprevious command
\tDown\t\tnext command
\tHome\t\tbeginning of line
\tEnd\t\tend of line
\tControl-a\tbeginning of line
\tControl-b\tbackward char
\tControl-c\tinterrupt command
\tControl-d\tdelete char
\tControl-e\tend of line
\tControl-f\t\tforward char
\tControl-h\tbackward delete char
\tControl-k\tdelete end of line
\tControl-n\tnext command
\tControl-p\tprevious command
\tControl-t\t\ttranspose
\tControl-u\tdelete line
\tControl-w\tbackward delete word" }
        }
.$id.menubar.file.pref.cle add radiobutton -value vi -variable mged_gui($id,edit_style)\
	-label "vi" -underline 0 -command "set_text_key_bindings $id"
hoc_register_menu_data "Command Line Edit" "vi" "Command Line Edit - Vi"\
	{ { synopsis "Set the command line edit mode to vi." }
          { description "
\t************ VI Insert Mode ************
\tEscape\t\tcommand
\tLeft\t\tbackward char, command
\tRight\t\tforward char, command
\tBackSpace\tbackward delete char

\t************ VI Command Mode ************
\tLeft\t\tbackward char
\tRight\t\tforward char
\tBackSpace\tbackward char
\tSpace\t\tforward char
\tA\t\tend of line, insert (i.e. append to end of line)
\tC\t\tdelete to end of line, insert
\tD\t\tdelete to end of line
\tF\t\tsearch backward char
\tI\t\tbeginning of line, insert
\tR\t\toverwrite
\tX\t\tbackward delete char
\t0\t\tbeginning of line
\t$\t\tend of line
\t;\t\tcontinue search in same direction
\t,\t\tcontinue search in opposite direction
\ta\t\tforward char, insert (i.e. append)
\tb\t\tbackward word
\tc\t\tchange
\td\t\tdelete
\te\t\tend of word
\tf\t\tsearch forward char
\th\t\tbackward char
\ti\t\tinsert
\tj\t\tnext command
\tk\t\tprevious command
\tl\t\tforward char
\tr\t\treplace char
\ts\t\tdelete char, insert
\tw\t\tforward word
\tx\t\tdelete char

\t************ Both ************
\tDelete\t\tbackward delete char
\tUp\t\tprevious command
\tDown\t\tnext command
\tHome\t\tbeginning of line
\tEnd\t\tend of line
\tControl-a\tbeginning of line
\tControl-b\tbackward char
\tControl-c\tinterrupt command
\tControl-e\tend of line
\tControl-f\t\tforward char
\tControl-h\tbackward delete char
\tControl-k\tdelete end of line
\tControl-n\tnext command
\tControl-p\tprevious command
\tControl-t\t\ttranspose
\tControl-u\tdelete to beginning of line
\tControl-w\tbackward delete word" }
        }

menu .$id.menubar.file.pref.special_chars -title "Special Characters" -tearoff $mged_default(tearoff_menus)
.$id.menubar.file.pref.special_chars add radiobutton -value 0 -variable glob_compat_mode\
	-label "Tcl Evaluation" -underline 0
hoc_register_menu_data "Special Characters" "Tcl Evaluation" "Tcl Evaluation"\
	{ { summary "Set the command interpretation mode to Tcl mode.
In this mode, globbing is not performed against
MGED database objects. Rather, the command string
is passed, unmodified, to the Tcl interpreter." } }
.$id.menubar.file.pref.special_chars add radiobutton -value 1 -variable glob_compat_mode\
	-label "Object Name Matching" -underline 0
hoc_register_menu_data "Special Characters" "Object Name Matching" "Object Name Matching"\
	{ { summary "Set the command interpretation mode to MGED object
name matching. In this mode, globbing is performed
against MGED database objects."\
        } }

menu .$id.menubar.edit -title "Edit" -tearoff $mged_default(tearoff_menus)
.$id.menubar.edit add command -label "Solid Selection..." -underline 0 \
	-command "winset \$mged_gui($id,active_dm); build_edit_menu_all s"
hoc_register_menu_data "Edit" "Solid Selection..." "Solid Selection"\
	{ { summary "A tool for selecting a solid to edit." } }
.$id.menubar.edit add command -label "Matrix Selection..." -underline 0 \
	-command "winset \$mged_gui($id,active_dm); build_edit_menu_all o"
hoc_register_menu_data "Edit" "Matrix Selection..." "Matrix Selection"\
	{ { summary "A tool for selecting a matrix to edit." } }
.$id.menubar.edit add separator
.$id.menubar.edit add command -label "Solid Editor..." -underline 6 \
	-command "init_edit_solid $id"
hoc_register_menu_data "Edit" "Solid Editor..." "Solid Editor"\
	{ { summary "A tool for editing/creating solids." } }
.$id.menubar.edit add command -label "Combination Editor..." -underline 0 \
	-command "init_comb $id"
hoc_register_menu_data "Edit" "Combination Editor..." "Combination Editor"\
	{ { summary "A tool for editing/creating combinations." } }


menu .$id.menubar.create -title "Create" -tearoff $mged_default(tearoff_menus)
.$id.menubar.create add cascade\
	-label "Make Solid" -underline 0 -menu .$id.menubar.create.solid
#.$id.menubar.create add command\
#	-label "Instance Creation Panel..." -underline 0 -command "icreate $id"
.$id.menubar.create add command -label "Solid Editor..." -underline 0 \
	-command "init_edit_solid $id"
hoc_register_menu_data "Create" "Solid Editor..." "Solid Editor"\
	{ { summary "A tool for editing/creating solids." } }
.$id.menubar.create add command -label "Combination Editor..." -underline 0 \
	-command "init_comb $id"
hoc_register_menu_data "Create" "Combination Editor..." "Combination Editor"\
	{ { summary "A tool for editing/creating combinations." } }

menu .$id.menubar.create.solid -title "Make Solid" -tearoff $mged_default(tearoff_menus)
set make_solid_types [_mged_make -t]
foreach solid_type $make_solid_types {
    .$id.menubar.create.solid add command -label "$solid_type..."\
	    -command "init_solid_create $id $solid_type"

    set ksl {}
    lappend ksl "summary \"Make a $solid_type using the values found in the tcl variable solid_data(attr,$solid_type).\"" "see_also \"make, in\""
    hoc_register_menu_data "Make Solid" "$solid_type..." "Make a $solid_type" $ksl
}

menu .$id.menubar.view -title "View" -tearoff $mged_default(tearoff_menus)
.$id.menubar.view add command -label "Top" -underline 0\
	-command "mged_apply $id \"press top\""
hoc_register_menu_data "View" "Top" "Top View"\
	{ { summary "View of the top (i.e. azimuth = 270, elevation = 90)." }
          { accelerator "t" }
          { see_also "press, ae, view, viewset, viewget" } }
.$id.menubar.view add command -label "Bottom" -underline 0\
	-command "mged_apply $id \"press bottom\""
hoc_register_menu_data "View" "Bottom" "Bottom View"\
	{ { summary "View of the bottom (i.e. azimuth = 270 , elevation = -90)." }
          { accelerator "b" }
          { see_also "press, ae, view, viewset, viewget" } }
.$id.menubar.view add command -label "Right" -underline 0\
	-command "mged_apply $id \"press right\""
hoc_register_menu_data "View" "Right" "Right View"\
	{ { summary "View of the right side (i.e. azimuth = 270, elevation = 0)." }
          { accelerator "r" }
          { see_also "press, ae, view, viewset, viewget" } }
.$id.menubar.view add command -label "Left" -underline 0\
	-command "mged_apply $id \"press left\""
hoc_register_menu_data "View" "Left" "Left View"\
	{ { summary "View of the left side (i.e. azimuth = 90, elevation = 0)." }
          { accelerator "l" }
          { see_also "press, ae, view, viewset, viewget" } }
.$id.menubar.view add command -label "Front" -underline 0\
	-command "mged_apply $id \"press front\""
hoc_register_menu_data "View" "Front" "Front View"\
	{ { summary "View of the front (i.e. azimuth = 0, elevation = 0)." }
          { accelerator "f" }
          { see_also "press, ae, view, viewset, viewget" } }
.$id.menubar.view add command -label "Rear" -underline 3\
	-command "mged_apply $id \"press rear\""
hoc_register_menu_data "View" "Rear" "Rear View"\
	{ { summary "View of the rear (i.e. azimuth = 180, elevation = 0)." }
          { accelerator "R" }
          { see_also "press, ae, view, viewset, viewget" } }
.$id.menubar.view add command -label "az35,el25" -underline 2\
	-command "mged_apply $id \"press 35,25\""
hoc_register_menu_data "View" "az35,el25" "View - 35,25"\
	{ { summary "View with an azimuth of 35 and an elevation of 25." }
          { accelerator "3" }
          { see_also "press, ae, view, viewset, viewget" } }
.$id.menubar.view add command -label "az45,el45" -underline 2\
	-command "mged_apply $id \"press 45,45\""
hoc_register_menu_data "View" "az45,el45" "View - 45,45"\
	{ { summary "View with an azimuth of 45 and an elevation of 45." }
          { accelerator "4" }
          { see_also "press, ae, view, viewset, viewget" } }
.$id.menubar.view add separator
.$id.menubar.view add command -label "Zoom In" -underline 5\
	-command "mged_apply $id \"zoom 2\""
hoc_register_menu_data "View" "Zoom In" "Zoom In"\
	{ { summary "Zoom in by a factor of 2." }
          { see_also "sca, zoom" } }
.$id.menubar.view add command -label "Zoom Out" -underline 5\
	-command "mged_apply $id \"zoom 0.5\""
hoc_register_menu_data "View" "Zoom Out" "Zoom Out"\
	{ { summary "Zoom out by a factor of 2." }
          { see_also "sca, zoom" } }
.$id.menubar.view add separator
.$id.menubar.view add command -label "Default"\
	-underline 0 -command "mged_apply $id \"press reset\""
hoc_register_menu_data "View" "Default" "Default View"\
	{ { summary "Same as top (i.e. azimuth = 270, elevation = 90)." }
          { see_also "press, ae, view, viewset, viewget" } }
.$id.menubar.view add command -label "Multipane Defaults"\
	-underline 0 -command "set_default_views $id"
hoc_register_menu_data "View" "Multipane Defaults" "Multipane Default Views"\
	{ { summary "Sets the view of all four panes to their defaults.\n
\tupper left\taz = 90, el = 0
\tupper right\taz = 35, el = 25
\tlower left\taz = 0, el = 0
\tlower right\taz = 90, el = 0" }
          { see_also "press, ae, view, viewset, viewget" } }
.$id.menubar.view add command -label "Zero" -underline 0\
	-command "mged_apply $id \"knob zero\""
hoc_register_menu_data "View" "Zero" "Zero Knobs"\
	{ { summary "Stop all rate transformations." }
          { accelerator "0" }
          { see_also "knob, press" } }

menu .$id.menubar.viewring -title "ViewRing" -tearoff $mged_default(tearoff_menus)
.$id.menubar.viewring add command -label "Add View" -underline 0 -command "view_ring_add $id"
hoc_register_menu_data "ViewRing" "Add View" "Add View"\
	{ { synopsis "Add a view to the view ring." }
          { description "A view ring is a mechanism for managing multiple
views within a single pane or display manager. Each pane
has its own view ring where any number of views can be stored.
The stored views can be removed or traversed." }
          { see_also "view_ring" } }
.$id.menubar.viewring add cascade -label "Select View" -underline 0 -menu .$id.menubar.viewring.select
.$id.menubar.viewring add cascade -label "Delete View" -underline 0 -menu .$id.menubar.viewring.delete
.$id.menubar.viewring add command -label "Next View" -underline 0 -command "view_ring_next $id"
hoc_register_menu_data "ViewRing" "Next View" "Next View"\
	{ { synopsis "Go to the next view on the view ring." }
          { accelerator "Control-n" }
          { see_also "view_ring" } }
.$id.menubar.viewring add command -label "Prev View" -underline 0 -command "view_ring_prev $id"
hoc_register_menu_data "ViewRing" "Prev View" "Previous View"\
	{ { synopsis "Go to the previous view on the view ring." }
          { accelerator "Control-p" }
          { see_also "view_ring" } }
.$id.menubar.viewring add command -label "Last View" -underline 0 -command "view_ring_toggle $id"
hoc_register_menu_data "ViewRing" "Last View" "Last View"\
	{ { synopsis "Go to the last view. This can be used to toggle
between two views." }
          { accelerator "Control-t" }
          { see_also "view_ring" } }

menu .$id.menubar.viewring.select -title "Select View" -tearoff $mged_default(tearoff_menus)\
	-postcommand "update_view_ring_labels $id"
update_view_ring_entries $id s
set view_ring($id) 1
menu .$id.menubar.viewring.delete -title "Delete View" -tearoff $mged_default(tearoff_menus)\
	-postcommand "update_view_ring_labels $id"
update_view_ring_entries $id d

menu .$id.menubar.settings -title "Settings" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings add cascade -label "Mouse Behavior" -underline 0\
	-menu .$id.menubar.settings.mouse_behavior
.$id.menubar.settings add cascade -label "Transform" -underline 0\
	-menu .$id.menubar.settings.transform
.$id.menubar.settings add cascade -label "Constraint Coords" -underline 0\
	-menu .$id.menubar.settings.coord
.$id.menubar.settings add cascade -label "Rotate About" -underline 0\
	-menu .$id.menubar.settings.origin
.$id.menubar.settings add cascade -label "Active Pane" -underline 0\
	-menu .$id.menubar.settings.mpane
.$id.menubar.settings add cascade -label "Apply To" -underline 1\
	-menu .$id.menubar.settings.applyTo
.$id.menubar.settings add cascade -label "Query Ray Effects" -underline 0\
	-menu .$id.menubar.settings.qray
.$id.menubar.settings add cascade -label "Grid" -underline 0\
	-menu .$id.menubar.settings.grid
.$id.menubar.settings add cascade -label "Grid Spacing" -underline 5\
	-menu .$id.menubar.settings.grid_spacing
.$id.menubar.settings add cascade -label "Framebuffer" -underline 0\
	-menu .$id.menubar.settings.fb
#.$id.menubar.settings add cascade -label "Pane Background Color" -underline 5\
#	-menu .$id.menubar.settings.bgColor
.$id.menubar.settings add cascade -label "View Axes Position" -underline 0\
	-menu .$id.menubar.settings.vap

menu .$id.menubar.settings.applyTo -title "Apply To" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.applyTo add radiobutton -value 0 -variable mged_gui($id,apply_to)\
	-label "Active Pane" -underline 0
hoc_register_menu_data "Apply To" "Active Pane" "Active Pane"\
	{ { summary "Set the \"Apply To\" mode such that the user's
interaction with the GUI is applied to the active pane." } }
.$id.menubar.settings.applyTo add radiobutton -value 1 -variable mged_gui($id,apply_to)\
	-label "Local Panes" -underline 0
hoc_register_menu_data "Apply To" "Local Panes" "Local Panes"\
	{ { summary "Set the \"Apply To\" mode such that the user's
interaction with the GUI is applied to all panes
local to this instance of the GUI." } }
.$id.menubar.settings.applyTo add radiobutton -value 2 -variable mged_gui($id,apply_to)\
	-label "Listed Panes" -underline 1
hoc_register_menu_data "Apply To" "Listed Panes" "Listed Panes"\
	{ { summary "Set the \"Apply To\" mode such that the user's
interaction with the GUI is applied to all panes
listed in the Tcl variable mged_gui(id,apply_list)
(Note - id refers to the GUI's id)." } }
.$id.menubar.settings.applyTo add radiobutton -value 3 -variable mged_gui($id,apply_to)\
	-label "All Panes" -underline 4
hoc_register_menu_data "Apply To" "All Panes" "All Panes"\
	{ { summary "Set the \"Apply To\" mode such that the user's
interaction with the GUI is applied to all panes." } }

menu .$id.menubar.settings.mouse_behavior -title "Mouse Behavior" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.mouse_behavior add radiobutton -value d -variable mged_gui($id,mouse_behavior)\
	-label "Default" -underline 0\
	-command "mged_apply $id \"set mouse_behavior \$mged_gui($id,mouse_behavior); refresh\""
hoc_register_menu_data "Mouse Behavior" "Default" "Default Mouse Behavior"\
	{ { synopsis "Enter the default MGED mouse behavior mode." }
          { description "In this mode, the user gets mouse behavior that is
the same as MGED 4.5 and earlier. See the table below.\n\n
\tMouse Button\t\t\tBehavior
\t\t1\t\tZoom out by a factor of 2
\t\t2\t\tCenter View
\t\t3\t\tZoom in by a factor of 2" }
          { see_also "rset, vars" } }
.$id.menubar.settings.mouse_behavior add radiobutton -value p -variable mged_gui($id,mouse_behavior)\
	-label "Paint Rectangle Area" -underline 0\
	-command "mged_apply $id \"set mouse_behavior \$mged_gui($id,mouse_behavior); refresh\""
hoc_register_menu_data "Mouse Behavior" "Paint Rectangle Area" "Paint Rectangle Area"\
	{ { synopsis "Enter paint rectangle mode." }
          { description "If the framebuffer is active, the rectangular area
as specified by the user is painted with the contents of the
framebuffer. Otherwise, only the rectangle is drawn.\n
\tMouse Button\t\t\tBehavior
\t\t1\t\tZoom out by a factor of 2
\t\t2\t\tDraw paint rectangle
\t\t3\t\tZoom in by a factor of 2" }
          { see_also "rset, vars" } }
.$id.menubar.settings.mouse_behavior add radiobutton -value r -variable mged_gui($id,mouse_behavior)\
	-label "Raytrace Rectangle Area" -underline 0\
	-command "mged_apply $id \"set mouse_behavior \$mged_gui($id,mouse_behavior); refresh\""
hoc_register_menu_data "Mouse Behavior" "Raytrace Rectangle Area" "Raytrace Rectangle Area"\
	{ { synopsis "Enter raytrace rectangle mode." }
          { description "If the framebuffer is active, the rectangular area as
specified by the user is raytraced. The rectangular area is
also painted with the contents of the framebuffer. Otherwise,
only the rectangle is drawn.\n
\tMouse Button\t\t\tBehavior
\t\t1\t\tZoom out by a factor of 2
\t\t2\t\tDraw raytrace rectangle
\t\t3\t\tZoom in by a factor of 2" }
          { see_also "rset, vars" } }
.$id.menubar.settings.mouse_behavior add radiobutton -value z -variable mged_gui($id,mouse_behavior)\
	-label "Zoom Rectangle Area" -underline 0\
	-command "mged_apply $id \"set mouse_behavior \$mged_gui($id,mouse_behavior); refresh\""
hoc_register_menu_data "Mouse Behavior" "Zoom Rectangle Area" "Zoom Rectangle Area"\
	{ { synopsis "Enter zoom rectangle mode." }
          { description "The rectangular area as specified by the user is used
to zoom the view. Note - as the user stretches out the rubber band
rectangle, the rectangle is constrained to be the same shape as the
window. This insures that the user gets what he or she sees.\n
\tMouse Button\t\t\tBehavior
\t\t1\t\tZoom out by a factor of 2
\t\t2\t\tDraw zoom rectangle
\t\t3\t\tZoom in by a factor of 2" }
          { see_also "rset, vars" } }
.$id.menubar.settings.mouse_behavior add radiobutton -value q -variable mged_gui($id,mouse_behavior)\
	-label "Query Ray" -underline 0\
	-command "mged_apply $id \"set mouse_behavior \$mged_gui($id,mouse_behavior); refresh\""
hoc_register_menu_data "Mouse Behavior" "Query Ray" "Query Ray"\
	{ { synopsis "Enter query ray mode." }
          { description "In this mode, the mouse is used to fire rays. The data
from the fired rays can be viewed textually, graphically
or both.\n
\tMouse Button\t\t\tBehavior
\t\t1\t\tZoom out by a factor of 2
\t\t2\t\tFire query ray
\t\t3\t\tZoom in by a factor of 2" }
          { see_also "nirt, qray, rset, vars" } }
.$id.menubar.settings.mouse_behavior add radiobutton -value s -variable mged_gui($id,mouse_behavior)\
	-label "Solid Edit Ray" -underline 0\
	-command "mged_apply $id \"set mouse_behavior \$mged_gui($id,mouse_behavior); refresh\""
hoc_register_menu_data "Mouse Behavior" "Solid Edit Ray" "Solid Edit Ray"\
	{ { synopsis "Enter solid edit ray mode." }
          { description "In this mode, the mouse is used to fire rays for selecting
a solid to edit. If more than one solid is hit, a listbox of the hit
solids is presented. The user then selects a solid to edit from
this listbox. If a single solid is hit, it is selected for editing.
If no solids were hit, a dialog is popped up saying that nothing
is hit. The user must then fire another ray to continue selecting
a solid. When a solid is finally selected, solid edit mode is
entered. When this happens, the mouse behavior mode is set to
default mode. Note - When selecting items from a listbox, a left
buttonpress highlights the solid in question until the button is
released. To select a solid, double click with the left mouse button.\n
\tMouse Button\t\t\tBehavior
\t\t1\t\tZoom out by a factor of 2
\t\t2\t\tFire solid edit ray
\t\t3\t\tZoom in by a factor of 2" }
          { see_also "nirt, qray, rset, sed, vars" } }
.$id.menubar.settings.mouse_behavior add radiobutton -value m -variable mged_gui($id,mouse_behavior)\
	-label "Matrix Edit Ray" -underline 0\
	-command "mged_apply $id \"set mouse_behavior \$mged_gui($id,mouse_behavior); refresh\""
hoc_register_menu_data "Mouse Behavior" "Matrix Edit Ray" "Matrix Edit Ray"\
	{ { synopsis "Enter matrix edit ray mode." }
          { description "In this mode, the mouse is used to fire rays for selecting
a matrix to edit. If more than one solid is hit, a listbox of the
hit solids is presented. The user then selects a solid
from this listbox. If a single solid is hit, that solid is selected.
If no solids were hit, a dialog is popped up saying that nothing is hit.
The user must then fire another ray to continue selecting a matrix to edit.
When a solid is finally selected, the user is presented with a listbox consisting
of the path components of the selected solid. From this lisbox, the
user selects a path component. This component determines which
matrix will be edited. After selecting the path component, object/matrix
edit mode is entered. When this happens, the mouse behavior mode
is set to default mode. Note - When selecting items
from a listbox, a left buttonpress highlights the solid/matrix in question
until the button is released. To select a solid/matrix, double click with
the left mouse button.\n
\tMouse Button\t\t\tBehavior
\t\t1\t\tZoom out by a factor of 2
\t\t2\t\tFire matrix edit ray
\t\t3\t\tZoom in by a factor of 2" }
          { see_also "rset, vars" } }
.$id.menubar.settings.mouse_behavior add radiobutton -value c -variable mged_gui($id,mouse_behavior)\
	-label "Combination Edit Ray" -underline 0\
	-command "mged_apply $id \"set mouse_behavior \$mged_gui($id,mouse_behavior); refresh\""
hoc_register_menu_data "Mouse Behavior" "Combination Edit Ray" "Combination Edit Ray"\
	{ { synopsis "Enter combination edit ray mode." }
        { description "In this mode, the mouse is used to fire rays for selecting
a combination to edit. If more than one combination is hit, a listbox of the
hit combinations is presented. The user then selects a combination
from this menu. If a single combination is hit, that combination is selected.
If no combinations were hit, a dialog is popped up saying that nothing is hit.
The user must then fire another ray to continue selecting a combination to edit.
When a combination is finally selected, the combination edit tool is presented
and initialized with the values of the selected combination. When this happens,
the mouse behavior mode is set to default mode. Note - When selecting items
from a menu, a left buttonpress highlights the combination in question
until the button is released. To select a combination, double click with
the left mouse button.\n
\tMouse Button\t\t\tBehavior
\t\t1\t\tZoom out by a factor of 2
\t\t2\t\tFire combination edit ray
\t\t3\t\tZoom in by a factor of 2" }
          { see_also "rset, vars" } }

menu .$id.menubar.settings.qray -title "Query Ray Effects" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.qray add radiobutton -value t -variable mged_gui($id,qray_effects)\
	-label "Text" -underline 0\
	-command "mged_apply $id \"qray effects \$mged_gui($id,qray_effects)\""
hoc_register_menu_data "Query Ray Effects" "Text" "Query Ray Effects - Text"\
	{ { summary "Set qray effects mode to 'text'. In this mode,
only textual output is used to represent the results
of firing a query ray." }
          { see_also "qray" } }
.$id.menubar.settings.qray add radiobutton -value g -variable mged_gui($id,qray_effects)\
	-label "Graphics" -underline 0\
	-command "mged_apply $id \"qray effects \$mged_gui($id,qray_effects)\""
hoc_register_menu_data "Query Ray Effects" "Graphics" "Query Ray Effects - Graphics"\
	{ { summary "Set qray effects mode to 'graphics'. In this mode,
only graphical output is used to represent the results
of firing a query ray." }
          { see_also "qray" } }
.$id.menubar.settings.qray add radiobutton -value b -variable mged_gui($id,qray_effects)\
	-label "both" -underline 0\
	-command "mged_apply $id \"qray effects \$mged_gui($id,qray_effects)\""
hoc_register_menu_data "Query Ray Effects" "both" "Query Ray Effects - Both"\
	{ { summary "Set qray effects mode to 'both'. In this mode,
both textual and graphical output is used to
represent the results of firing a query ray." }
          { see_also "qray" } }

menu .$id.menubar.settings.mpane -title "Active Pane" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.mpane add radiobutton -value ul -variable mged_gui($id,dm_loc)\
	-label "Upper Left" -underline 6\
	-command "set_active_dm $id"
hoc_register_menu_data "Active Pane" "Upper Left" "Active Pane - Upper Left"\
	{ { summary "Set the active pane to be the upper left pane.
Any interaction with the GUI that affects a pane
(display manager) will be directed at the upper
left pane." } }
.$id.menubar.settings.mpane add radiobutton -value ur -variable mged_gui($id,dm_loc)\
	-label "Upper Right" -underline 6\
	-command "set_active_dm $id"
hoc_register_menu_data "Active Pane" "Upper Right" "Active Pane - Upper Right"\
	{ { summary "Set the active pane to be the upper right pane.
Any interaction with the GUI that affects a pane
(display manager) will be directed at the upper
right pane." } }
.$id.menubar.settings.mpane add radiobutton -value ll -variable mged_gui($id,dm_loc)\
	-label "Lower Left" -underline 2\
	-command "set_active_dm $id"
hoc_register_menu_data "Active Pane" "Lower Left" "Active Pane - Lower Left"\
	{ { summary "Set the active pane to be the lower left pane.
Any interaction with the GUI that affects a pane
(display manager) will be directed at the lower
left pane." } }
.$id.menubar.settings.mpane add radiobutton -value lr -variable mged_gui($id,dm_loc)\
	-label "Lower Right" -underline 3\
	-command "set_active_dm $id"
hoc_register_menu_data "Active Pane" "Lower Right" "Active Pane - Lower Right"\
	{ { summary "Set the active pane to be the lower right pane.
Any interaction with the GUI that affects a pane
(geometry window) will be directed at the lower
right pane." } }

menu .$id.menubar.settings.fb -title "Framebuffer" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.fb add radiobutton -value 1 -variable mged_gui($id,fb_all)\
	-label "All" -underline 0\
	-command "mged_apply $id \"set fb_all \$mged_gui($id,fb_all)\""
hoc_register_menu_data "Framebuffer" "All" "Framebuffer - All"\
	{ { summary "Use the entire pane for the framebuffer." }
          { see_also "rset, vars" } }
.$id.menubar.settings.fb add radiobutton -value 0 -variable mged_gui($id,fb_all)\
	-label "Rectangle Area" -underline 0\
	-command "mged_apply $id \"set fb_all \$mged_gui($id,fb_all)\""
hoc_register_menu_data "Framebuffer" "Rectangle Area" "Framebuffer - Rectangle Area"\
	{ { summary "Use the rectangle area for the framebuffer." }
          { see_also "rset, vars" } }
.$id.menubar.settings.fb add separator
.$id.menubar.settings.fb add radiobutton -value 1 -variable mged_gui($id,fb_overlay)\
	-label "Overlay" -underline 0\
	-command "mged_apply $id \"set fb_overlay \$mged_gui($id,fb_overlay)\""
hoc_register_menu_data "Framebuffer" "Overlay" "Framebuffer - Overlay"\
	{ { summary "Put the framebuffer in overlay mode. In this mode,
the framebuffer data is placed in the pane after
the geometry is drawn (i.e. the framebuffer data is
is drawn on top of the geometry)." }
          { see_also "rset, vars" } }
.$id.menubar.settings.fb add radiobutton -value 0 -variable mged_gui($id,fb_overlay)\
	-label "Underlay" -underline 0\
	-command "mged_apply $id \"set fb_overlay \$mged_gui($id,fb_overlay)\""
hoc_register_menu_data "Framebuffer" "Underlay" "Framebuffer - Underlay"\
	{ { summary "Put the framebuffer in underlay mode. In this mode,
the framebuffer data is placed in the pane before
the geometry is drawn (i.e. the geometry is drawn on
top of the framebuffer data)." }
          { see_also "rset, vars" } }
.$id.menubar.settings.fb add separator
.$id.menubar.settings.fb add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,fb)\
	-label "Framebuffer Active" -underline 0\
	-command "set_fb $id; update_Raytrace $id"
hoc_register_menu_data "Framebuffer" "Framebuffer Active" "Framebuffer Active"\
	{ { summary "This activates/deactivates the framebuffer." }
          { see_also "rset, vars" } }
.$id.menubar.settings.fb add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,listen)\
	-label "Listen For Clients" -underline 0\
	-command "set_listen $id" -state disabled
hoc_register_menu_data "Framebuffer" "Listen For Clients" "Listen For Clients"\
	{ { summary "This activates/deactivates listening for clients.
If the framebuffer is listening for clients, new
data can be passed into the framebuffer. Otherwise,
the framebuffer is write protected. Actually, it is
also read protected. The one exception is that MGED
can still read the framebuffer data." }
          { see_also "rset, vars" } }

menu .$id.menubar.settings.vap -title "View Axes Position" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.vap add radiobutton -value 0 -variable mged_gui($id,view_pos)\
	-label "Center" -underline 0\
	-command "mged_apply $id \"rset ax view_pos 0 0\""
hoc_register_menu_data "View Axes Position" "Center" "View Axes Position - Center"\
	{ { summary "Place the view axes in the center of the active pane." }
          { see_also "rset" } }
.$id.menubar.settings.vap add radiobutton -value 1 -variable mged_gui($id,view_pos)\
	-label "Lower Left" -underline 2\
	-command "mged_apply $id \"rset ax view_pos -1750 -1750\""
hoc_register_menu_data "View Axes Position" "Lower Left" "View Axes Position - Lower Left"\
	{ { summary "Place the view axes in the lower left corner of the active pane." }
          { see_also "rset" } }
.$id.menubar.settings.vap add radiobutton -value 2 -variable mged_gui($id,view_pos)\
	-label "Upper Left" -underline 6\
	-command "mged_apply $id \"rset ax view_pos -1750 1750\""
hoc_register_menu_data "View Axes Position" "Upper Left" "View Axes Position - Upper Left"\
	{ { summary "Place the view axes in the upper left corner of the active pane." }
          { see_also "rset" } }
.$id.menubar.settings.vap add radiobutton -value 3 -variable mged_gui($id,view_pos)\
	-label "Upper Right" -underline 6\
	-command "mged_apply $id \"rset ax view_pos 1750 1750\""
hoc_register_menu_data "View Axes Position" "Upper Right" "View Axes Position - Upper Right"\
	{ { summary "Place the view axes in the upper right corner of the active pane." }
          { see_also "rset" } }
.$id.menubar.settings.vap add radiobutton -value 4 -variable mged_gui($id,view_pos)\
	-label "Lower Right" -underline 3\
	-command "mged_apply $id \"rset ax view_pos 1750 -1750\""
hoc_register_menu_data "View Axes Position" "Lower Right" "View Axes Position - Lower Right"\
	{ { summary "Place the view axes in the lower right corner of the active pane." }
          { see_also "rset" } }

menu .$id.menubar.settings.grid -title "Grid" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.grid add command -label "Anchor..." -underline 0\
	-command "do_grid_anchor $id"
hoc_register_menu_data "Grid" "Anchor..." "Grid Anchor"\
	{ { summary "Pops up the grid anchor entry dialog." }
          { see_also "rset" } }
.$id.menubar.settings.grid add cascade -label "Spacing" -underline 1\
	-menu .$id.menubar.settings.grid.spacing
.$id.menubar.settings.grid add separator
.$id.menubar.settings.grid add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,grid_draw)\
	-label "Draw Grid" -underline 0\
	-command "mged_apply $id \"rset grid draw \$mged_gui($id,grid_draw)\""
hoc_register_menu_data "Grid" "Draw Grid" "Draw Grid"\
	{ { summary "Toggle drawing the grid. The grid is a lattice
of points over the pane (geometry window). The
regular spacing between the points gives the user
accurate visual cues regarding dimension. This
spacing can be set by the user." }
          { see_also "rset" } }
.$id.menubar.settings.grid add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,draw_snap)\
	-label "Snap To Grid" -underline 0\
	-command "mged_apply $id \"rset grid snap \$mged_gui($id,draw_snap)\""
hoc_register_menu_data "Grid" "Snap To Grid" "Snap To Grid"\
	{ { summary "Toggle grid snapping. When snapping to grid,
the internal routines that use the mouse pointer
location, move/snap that location to the nearest
grid point. This gives the user high accuracy with
the mouse for transforming the view or editing
solids/matrices." }
          { see_also "rset" } }

menu .$id.menubar.settings.grid.spacing -title "Grid Spacing" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.grid.spacing add command -label "Autosize" -underline 0\
	-command "grid_spacing_autosize $id; grid_spacing_apply $id b"
hoc_register_menu_data "Grid Spacing" "Autosize" "Grid Spacing - Autosize"\
	{ { summary "Set the grid spacing according to the current view size.
The number of ticks will be between 20 and 200 in user units.
The major spacing will be set to 10 ticks per major." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "Arbitrary..." -underline 1\
	-command "do_grid_spacing $id b"
hoc_register_menu_data "Grid Spacing" "Arbitrary..." "Grid Spacing - Arbitrary"\
	{ { summary "Pops up the grid spacing entry dialog. The user
can use this to set both the horizontal and
vertical tick spacing." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add separator
.$id.menubar.settings.grid.spacing add command -label "micrometer" -underline 4\
	-command "set_grid_spacing $id micrometer 1"
hoc_register_menu_data "Grid Spacing" "micrometer" "Grid Spacing - micrometer"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1 micrometer." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "millimeter" -underline 2\
	-command "set_grid_spacing $id millimeter 1"
hoc_register_menu_data "Grid Spacing" "millimeter" "Grid Spacing - millimeter"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1 millimeter." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "centimeter" -underline 0\
	-command "set_grid_spacing $id centimeter 1"
hoc_register_menu_data "Grid Spacing" "centimeter" "Grid Spacing - centimeter"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1 centimeter." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "decimeter" -underline 0\
	-command "set_grid_spacing $id decimeter 1"
hoc_register_menu_data "Grid Spacing" "decimeter" "Grid Spacing - decimeter"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1 decimeter." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "meter" -underline 0\
	-command "set_grid_spacing $id meter 1"
hoc_register_menu_data "Grid Spacing" "meter" "Grid Spacing - meter"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1 meter." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "kilometer" -underline 0\
	-command "set_grid_spacing $id kilometer 1"
hoc_register_menu_data "Grid Spacing" "kilometer" "Grid Spacing - kilometer"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1 kilometer." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add separator
.$id.menubar.settings.grid.spacing add command -label "1/10 inch" -underline 0\
	-command "set_grid_spacing $id \"1/10 inch\" 1"
hoc_register_menu_data "Grid Spacing" "1/10 inch" "Grid Spacing - 1/10 inch"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1/10 inches." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "1/4 inch" -underline 2\
	-command "set_grid_spacing $id \"1/4 inch\" 1"
hoc_register_menu_data "Grid Spacing" "1/4 inch" "Grid Spacing - 1/4 inch"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1/4 inches." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "1/2 inch" -underline 2\
	-command "set_grid_spacing $id \"1/2 inch\" 1"
hoc_register_menu_data "Grid Spacing" "1/2 inch" "Grid Spacing - 1/2 inch"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1/2 inches." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "inch" -underline 0\
	-command "set_grid_spacing $id inch 1"
hoc_register_menu_data "Grid Spacing" "inch" "Grid Spacing - inch"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1 inch." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "foot" -underline 0\
	-command "set_grid_spacing $id foot 1"
hoc_register_menu_data "Grid Spacing" "foot" "Grid Spacing - foot"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1 foot." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "yard" -underline 0\
	-command "set_grid_spacing $id yard 1"
hoc_register_menu_data "Grid Spacing" "yard" "Grid Spacing - yard"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1 yard." }
          { see_also "rset" } }
.$id.menubar.settings.grid.spacing add command -label "mile" -underline 0\
	-command "set_grid_spacing $id mile 1"
hoc_register_menu_data "Grid Spacing" "mile" "mile"\
	{ { summary "Set the horizontal and vertical tick
spacing to 1 mile." }
          { see_also "rset" } }

#
# Don't need to register menu data here. Already being done above.
#
menu .$id.menubar.settings.grid_spacing -title "Grid Spacing" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.grid_spacing add command -label "Autosize" -underline 0\
	-command "grid_spacing_autosize $id; grid_spacing_apply $id b"
.$id.menubar.settings.grid_spacing add command -label "Arbitrary..." -underline 1\
	-command "do_grid_spacing $id b"
.$id.menubar.settings.grid_spacing add separator
.$id.menubar.settings.grid_spacing add command -label "micrometer" -underline 4\
	-command "set_grid_spacing $id micrometer 1"
.$id.menubar.settings.grid_spacing add command -label "millimeter" -underline 2\
	-command "set_grid_spacing $id millimeter 1"
.$id.menubar.settings.grid_spacing add command -label "centimeter" -underline 0\
	-command "set_grid_spacing $id centimeter 1"
.$id.menubar.settings.grid_spacing add command -label "decimeter" -underline 0\
	-command "set_grid_spacing $id decimeter 1"
.$id.menubar.settings.grid_spacing add command -label "meter" -underline 0\
	-command "set_grid_spacing $id meter 1"
.$id.menubar.settings.grid_spacing add command -label "kilometer" -underline 0\
	-command "set_grid_spacing $id kilometer 1"
.$id.menubar.settings.grid_spacing add separator
.$id.menubar.settings.grid_spacing add command -label "1/10 inch" -underline 0\
	-command "set_grid_spacing $id \"1/10 inch\" 1"
.$id.menubar.settings.grid_spacing add command -label "1/4 inch" -underline 2\
	-command "set_grid_spacing $id \"1/4 inch\" 1"
.$id.menubar.settings.grid_spacing add command -label "1/2 inch" -underline 2\
	-command "set_grid_spacing $id \"1/2 inch\" 1"
.$id.menubar.settings.grid_spacing add command -label "inch" -underline 0\
	-command "set_grid_spacing $id inch 1"
.$id.menubar.settings.grid_spacing add command -label "foot" -underline 0\
	-command "set_grid_spacing $id foot 1"
.$id.menubar.settings.grid_spacing add command -label "yard" -underline 0\
	-command "set_grid_spacing $id yard 1"
.$id.menubar.settings.grid_spacing add command -label "mile" -underline 0\
	-command "set_grid_spacing $id mile 1"

menu .$id.menubar.settings.coord -title "Constraint Coords" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.coord add radiobutton -value m -variable mged_gui($id,coords)\
	-label "Model" -underline 0\
	-command "mged_apply $id \"set coords \$mged_gui($id,coords)\""
hoc_register_menu_data "Constraint Coords" "Model" "Constraint Coords - Model"\
	{ { summary "Constrained transformations will use model coordinates." }
          { see_also "rset, vars" } }
.$id.menubar.settings.coord add radiobutton -value v -variable mged_gui($id,coords)\
	-label "View" -underline 0\
	-command "mged_apply $id \"set coords \$mged_gui($id,coords)\""
hoc_register_menu_data "Constraint Coords" "View" "Constraint Coords - View"\
	{ { summary "Constrained transformations will use view coordinates." }
          { see_also "rset, vars" } }
.$id.menubar.settings.coord add radiobutton -value o -variable mged_gui($id,coords)\
	-label "Object" -underline 0\
	-command "mged_apply $id \"set coords \$mged_gui($id,coords)\"" -state disabled
hoc_register_menu_data "Constraint Coords" "Object" "Constraint Coords - Object"\
	{ { summary "Constrained transformations will use object coordinates." }
          { see_also "rset, vars" } }

menu .$id.menubar.settings.origin -title "Rotate About" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.origin add radiobutton -value v -variable mged_gui($id,rotate_about)\
	-label "View Center" -underline 0\
	-command "mged_apply $id \"set rotate_about \$mged_gui($id,rotate_about)\""
hoc_register_menu_data "Rotate About" "View Center" "Rotate About - View Center"\
	{ { summary "Set the center of rotation to be about the view center." }
          { see_also "rset, vars" } }
.$id.menubar.settings.origin add radiobutton -value e -variable mged_gui($id,rotate_about)\
	-label "Eye" -underline 0\
	-command "mged_apply $id \"set rotate_about \$mged_gui($id,rotate_about)\""
hoc_register_menu_data "Rotate About" "Eye" "Rotate About - Eye"\
	{ { summary "Set the center of rotation to be about the eye." }
          { see_also "rset, vars" } }
.$id.menubar.settings.origin add radiobutton -value m -variable mged_gui($id,rotate_about)\
	-label "Model Origin" -underline 0\
	-command "mged_apply $id \"set rotate_about \$mged_gui($id,rotate_about)\""
hoc_register_menu_data "Rotate About" "Model Origin" "Rotate About - Model Origin"\
	{ { summary "Set the center of rotation to be about the model origin." }
          { see_also "rset, vars" } }
.$id.menubar.settings.origin add radiobutton -value k -variable mged_gui($id,rotate_about)\
	-label "Key Point" -underline 0\
	-command "mged_apply $id \"set rotate_about \$mged_gui($id,rotate_about)\"" -state disabled
hoc_register_menu_data "Rotate About" "Key Point" "Rotate About - Key Point"\
	{ { summary "Set the center of rotation to be about the key point." }
          { see_also "rset, vars" } }

menu .$id.menubar.settings.transform -title "Transform" -tearoff $mged_default(tearoff_menus)
.$id.menubar.settings.transform add radiobutton -value v -variable mged_gui($id,transform)\
	-label "View" -underline 0\
	-command "set_transform $id"
hoc_register_menu_data "Transform" "View" "Transform - View"\
	{ { summary "Set the transform mode to VIEW. When in VIEW mode, the mouse
can be used to transform the view. This is the default." }
          { see_also "rset, vars" } }
.$id.menubar.settings.transform add radiobutton -value a -variable mged_gui($id,transform)\
	-label "ADC" -underline 0\
	-command "set_transform $id"
hoc_register_menu_data "Transform" "ADC" "Transform - ADC"\
	{ { summary "Set the transform mode to ADC. When in ADC mode, the mouse
can be used to transform the angle distance cursor while the
angle distance cursor is being displayed. If the angle distance
cursor is not being displayed, the behavior is the same as VIEW
mode." }
          { see_also "rset, vars" } }
.$id.menubar.settings.transform add radiobutton -value e -variable mged_gui($id,transform)\
	-label "Model Params" -underline 0\
	-command "set_transform $id" -state disabled
hoc_register_menu_data "Transform" "Object Params" "Transform - Object Params"\
	{ { summary "Set the transform mode to OBJECT PARAMS. When in OBJECT PARAMS
mode, the mouse can be used to transform the object parameters." }
          { see_also "rset, vars" } }

menu .$id.menubar.modes -title "Modes" -tearoff $mged_default(tearoff_menus)
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,grid_draw)\
	-label "Draw Grid" -underline 0\
	-command "mged_apply $id \"rset grid draw \$mged_gui($id,grid_draw)\""
hoc_register_menu_data "Modes" "Draw Grid" "Draw Grid"\
	{ { summary "Toggle drawing the grid. The grid is a lattice
of points over the pane (geometry window). The
regular spacing between the points gives the user
accurate visual cues regarding dimension. This
spacing can be set by the user." }
          { see_also "rset" } }
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,draw_snap)\
	-label "Snap To Grid" -underline 0\
	-command "mged_apply $id \"rset grid snap \$mged_gui($id,draw_snap)\""
hoc_register_menu_data "Modes" "Snap To Grid" "Snap To Grid"\
	{ { summary "Toggle grid snapping. When snapping to grid,
the internal routines that use the mouse pointer
location, move/snap that location to the nearest
grid point. This gives the user high accuracy with
the mouse for transforming the view or editing
solids/matrices." }
          { see_also "rset" } }
.$id.menubar.modes add separator
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,fb)\
	-label "Framebuffer Active" -underline 0 \
	-command "set_fb $id; update_Raytrace $id"
hoc_register_menu_data "Modes" "Framebuffer Active" "Framebuffer Active"\
	{ { summary "This activates/deactivates the framebuffer." }
          { see_also "rset, vars" } }
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,listen)\
	-label "Listen For Clients" -underline 0\
	-command "set_listen $id" -state disabled
hoc_register_menu_data "Modes" "Listen For Clients" "Listen For Clients"\
	{ { summary "
This activates/deactivates listening for clients. If
the framebuffer is listening for clients, new data can
be passed into the framebuffer. Otherwise, the framebuffer
is write protected. Actually, it is also read protected
with one exception. MGED can still read the framebuffer
data." }
          { see_also "rset, vars" } }
.$id.menubar.modes add separator
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,rubber_band)\
	-label "Persistent Rubber Band" -underline 0\
	-command "mged_apply $id \"rset rb draw \$mged_gui($id,rubber_band)\""
hoc_register_menu_data "Modes" "Persistent Rubber Band" "Persistent Rubber Band"\
	{ { summary "Toggle drawing the rubber band while idle." }
          { see_also "rset" } }
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,adc_draw)\
	-label "Angle/Dist Cursor" -underline 0 \
	-command "mged_apply $id \"adc draw \$mged_gui($id,adc_draw)\""
hoc_register_menu_data "Modes" "Angle/Dist Cursor" "Angle/Dist Cursor"\
	{ { summary "Toggle drawing the angle distance cursor." }
          { see_also "adc" } }
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,faceplate)\
	-label "Faceplate" -underline 7\
	-command "mged_apply $id \"set faceplate \$mged_gui($id,faceplate)\""
hoc_register_menu_data "Modes" "Faceplate" "Faceplate"\
	{ { summary "Toggle drawing the MGED faceplate." }
          { see_also "rset, vars" } }
.$id.menubar.modes add cascade -label "Axes" -underline 1\
	-menu .$id.menubar.modes.axes
.$id.menubar.modes add separator
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,multi_view)\
	-label "Multipane" -underline 0 -command "setmv $id"
hoc_register_menu_data "Modes" "Multipane" "Multipane"\
	{ { summary "
Toggle between multipane and single pane mode.
In multipane mode there are four panes, each
with its own state." } }
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,show_edit_info)\
	-label "Edit Info" -underline 0\
	-command "toggle_edit_info $id"
hoc_register_menu_data "Modes" "Edit Info" "Edit Info"\
	{ { summary "
Toggle display of edit information. If in
solid edit state, the edit information is
displayed in the internal solid editor. This
editor, as its name implies, can be used to
edit the solid as well as view its contents.
If in object edit state, the object information
is displayed in a dialog box." } }
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,show_status)\
	-label "Status Bar" -underline 7\
	-command "toggle_status_bar $id"
hoc_register_menu_data "Modes" "Status Bar" "Status Bar"\
	{ { summary "Toggle display of the command window's status bar." } }
if {$comb} {
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,show_cmd)\
	-label "Command Window" -underline 0\
	-command "set_cmd_win $id"
hoc_register_menu_data "Modes" "Command Window" "Command Window"\
	{ { summary "Toggle display of the command window." } }
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,show_dm)\
	-label "Graphics Window" -underline 0\
	-command "set_dm_win $id"
hoc_register_menu_data "Modes" "Graphics Window" "Graphics Window"\
	{ { summary "Toggle display of the graphics window." } }
} 
.$id.menubar.modes add separator
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,rateknobs)\
	-label "Rateknobs" -underline 0\
	-command "mged_apply $id \"set rateknobs \$mged_gui($id,rateknobs)\""
hoc_register_menu_data "Modes" "Rateknobs" "Rate Knobs"\
	{ { summary "Toggle rate knob mode. When in rate knob mode,
transformation with the mouse becomes rate based.
For example, if the user rotates the view about
the X axis, the view continues to rotate about the
X axis until the rate rotation is stopped." }
          { see_also "knob" } }
.$id.menubar.modes add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,dlist)\
	-label "Display Lists" -underline 8\
	-command "mged_apply $id \"set dlist \$mged_gui($id,dlist)\""
hoc_register_menu_data "Modes" "Display Lists" "Display Lists"\
	{ { summary "Toggle the use of display lists. This currently affects
only Ogl display managers. When using display lists the
screen update time is significantly faster. This is especially
noticable when running MGED remotely. Use of display lists
is encouraged unless the geometry being viewed is bigger
than the Ogl server can handle (i.e. the server runs out
of available memory for storing display lists). When this
happens the machine will begin to swap (and little else).
If huge pieces of geometry need to be viewed, consider
toggling off display lists. Note that using display lists
while viewing geometry of any significant size will incur
noticable compute time up front to create the display lists."} }

menu .$id.menubar.modes.axes -title "Axes" -tearoff $mged_default(tearoff_menus)
.$id.menubar.modes.axes add checkbutton -offvalue 0 -onvalue 1\
	-variable mged_gui($id,view_draw) -label "View" -underline 0\
	-command "mged_apply $id \"rset ax view_draw \$mged_gui($id,view_draw)\""
hoc_register_menu_data "Axes" "View" "View Axes"\
	{ { summary "Toggle display of the view axes. The view axes
are used to give the user visual cues indicating
the current view of model space. These axes are
drawn the same as the model axes, except that the
view axes' position is fixed in view space. This
position as well as other characteristics can be
set by the user." }
          { see_also "rset" } }
.$id.menubar.modes.axes add checkbutton -offvalue 0 -onvalue 1\
	-variable mged_gui($id,model_draw) -label "Model" -underline 0\
	-command "mged_apply $id \"rset ax model_draw \$mged_gui($id,model_draw)\""
hoc_register_menu_data "Axes" "Model" "Model Axes"\
	{ { summary "Toggle display of the model axes. The model axes
are used to give the user visual cues indicating
the current view of model space. The model axes
are by default located at the model origin and are
fixed in model space. So, if the user transforms
the view, the model axes will move with respect to
the view. The model axes position as well as other
characteristics can be set by the user." }
          { see_also "rset" } }
.$id.menubar.modes.axes add checkbutton -offvalue 0 -onvalue 1\
	-variable mged_gui($id,edit_draw) -label "Edit" -underline 0\
	-command "mged_apply $id \"rset ax edit_draw \$mged_gui($id,edit_draw)\""
hoc_register_menu_data "Axes" "Edit" "Edit Axes"\
	{ { summary "Toggle display of the edit axes. The edit axes
are used to give the user visual cues indicating
how the edit is progressing. They consist of a
pair of axes. One remains unmoved, while the other
moves to indicate how things have changed." }
          { see_also "rset" } }

menu .$id.menubar.misc -title "Misc" -tearoff $mged_default(tearoff_menus)
.$id.menubar.misc add checkbutton -offvalue 0 -onvalue 1\
	-variable mged_gui($id,zclip) -label "Z Clipping" -underline 0\
	-command "mged_apply $id \"dm set zclip \$mged_gui($id,zclip)\""
hoc_register_menu_data "Misc" "Z Clipping" "Z Clipping"\
	{ { summary "Toggle zclipping. When zclipping is active, the Z value
of each point is checked against the min and max Z values
of the viewing cube. If the Z value of the point is found
outside this range, it is clipped (i.e. not drawn).
Zclipping can be used to remove geometric detail that may
be occluding geometry of greater interest." }
          { see_also "dm" } }
.$id.menubar.misc add checkbutton -offvalue 0 -onvalue 1\
	-variable mged_gui($id,perspective_mode) -label "Perspective" -underline 0\
	-command "mged_apply $id \"set perspective_mode \$mged_gui($id,perspective_mode)\""
hoc_register_menu_data "Misc" "Perspective" "Perspective"\
	{ { summary "Toggle perspective_mode."}
          { see_also "rset, vars" } }
.$id.menubar.misc add checkbutton -offvalue 0 -onvalue 1\
	-variable mged_gui($id,faceplate) -label "Faceplate" -underline 0\
	-command "mged_apply $id \"set faceplate \$mged_gui($id,faceplate)\""
hoc_register_menu_data "Misc" "Faceplate" "Faceplate"\
	{ { summary "Toggle drawing the MGED faceplate." }
          { see_also "rset, vars" } }
.$id.menubar.misc add checkbutton -offvalue 0 -onvalue 1\
	-variable mged_gui($id,orig_gui) -label "Faceplate GUI" -underline 10\
	-command "mged_apply $id \"set orig_gui \$mged_gui($id,orig_gui)\""
hoc_register_menu_data "Misc" "Faceplate GUI" "Faceplate GUI"\
	{ { summary "Toggle drawing the MGED faceplate GUI. The faceplate GUI
consists of the faceplate menu and sliders." }
          { see_also "rset, vars" } }
.$id.menubar.misc add checkbutton -offvalue 0 -onvalue 1\
	-variable mged_gui($id,forward_keys) -label "Keystroke Forwarding" -underline 8\
	-command "mged_apply $id \"set_forward_keys \\\[winset\\\] \$mged_gui($id,forward_keys)\""
hoc_register_menu_data "Misc" "Keystroke Forwarding" "Keystroke Forwarding"\
	{ { summary "Toggle keystroke forwarding. When forwarding keystrokes, each
key event within the drawing window is forwarded to the command
window. Drawing window specific commands (i.e. commands that
modify the state of the drawing window) will apply only to the
drawing window wherein the user typed. This feature is provided
to lessen the need to use the mouse." } }
if {$mged_gui($id,dtype) == "ogl"} {
    .$id.menubar.misc add checkbutton -offvalue 0 -onvalue 1\
	    -variable mged_gui($id,depthcue) -label "Depth Cueing" -underline 0\
	    -command "mged_apply $id \"dm set depthcue \$mged_gui($id,depthcue)\""
    hoc_register_menu_data "Misc" "Depth Cueing" "Depth Cueing"\
	    { { summary "Toggle depth cueing. When depth cueing is active,
lines that are farther away appear more faint." } 
    { see_also "dm" } }
    .$id.menubar.misc add checkbutton -offvalue 0 -onvalue 1\
	    -variable mged_gui($id,zbuffer) -label "Z Buffer" -underline 2\
	    -command "mged_apply $id \"dm set zbuffer \$mged_gui($id,zbuffer)\""
    hoc_register_menu_data "Misc" "Z Buffer" "Z Buffer"\
	    { { summary "Toggle Z buffer." }
              { see_also "dm" } }
    .$id.menubar.misc add checkbutton -offvalue 0 -onvalue 1\
	    -variable mged_gui($id,lighting) -label "Lighting" -underline 0\
	    -command "mged_apply $id \"dm set lighting \$mged_gui($id,lighting)\""
    hoc_register_menu_data "Misc" "Lighting" "Lighting"\
	    { { summary "Toggle lighting." }
              { see_also "dm" } }
}

menu .$id.menubar.tools -title "Tools" -tearoff $mged_default(tearoff_menus)
.$id.menubar.tools add command -label "ADC Control Panel..." -underline 0\
	-command "init_adc_control $id"
hoc_register_menu_data "Tools" "ADC Control Panel..." "ADC Control Panel"\
	{ { summary "Tool for controlling the angle distance cursor." }
          { see_also "adc" } }
.$id.menubar.tools add command -label "Grid Control Panel..." -underline 0\
	-command "init_grid_control $id"
hoc_register_menu_data "Tools" "Grid Control Panel..." "Grid Control Panel"\
	{ { summary "Tool for setting grid parameters." }
          { see_also "rset" } }
.$id.menubar.tools add command -label "Query Ray Control Panel..." -underline 0\
	-command "init_qray_control $id"
hoc_register_menu_data "Tools" "Query Ray Control Panel..." "Query Ray Control Panel"\
	{ { summary "Tool for setting query ray parameters." } }
.$id.menubar.tools add command -label "Raytrace Control Panel..." -underline 0\
	-command "init_Raytrace $id"
hoc_register_menu_data "Tools" "Raytrace Control Panel..." "Raytrace Control Panel"\
	{ { summary "Tool for raytracing the current view." }
          { see_also rt } }
.$id.menubar.tools add command -label "Animmate Control Panel..." -underline 1\
	-command "animmate .$id"
hoc_register_menu_data "Tools" "Animmate Control Panel..." "Animmate Control Panel"\
	{ { summary "Tool for building animation scripts." }
          { see_also animmate } }
.$id.menubar.tools add separator
.$id.menubar.tools add command -label "Solid Editor..." -underline 0\
	-command "init_edit_solid $id"
hoc_register_menu_data "Tools" "Solid Editor..." "Solid Editor"\
	{ { summary "Tool for creating/editing solids." } }
.$id.menubar.tools add command -label "Combination Editor..." -underline 0\
	-command "init_comb $id"
hoc_register_menu_data "Tools" "Combination Editor..." "Combination Editor"\
	{ { summary "Tool for creating/editing combinations." } }
.$id.menubar.tools add command -label "Color Editor..." -underline 1\
	-command "cadColorWidget tool .$id colorEditTool\
	-title \"Color Editor\"\
	-initialcolor black"
hoc_register_menu_data "Tools" "Color Editor..." "Color Editor"\
	{ { summary "Tool for displaying colors." } }
#.$id.menubar.tools add separator
#.$id.menubar.tools add checkbutton -offvalue 0 -onvalue 1 -variable mged_gui($id,classic_buttons)\
#	-label "Classic Menu Tool..." -underline 0\
#	-command "toggle_button_menu $id"

menu .$id.menubar.help -title "Help" -tearoff $mged_default(tearoff_menus)
.$id.menubar.help add command -label "About" -underline 0\
	-command "do_About_MGED $id"
hoc_register_menu_data "Help" "About" "About MGED"\
	{ { summary "Information about MGED" } }
.$id.menubar.help add command -label "Commands..." -underline 0\
	-command "command_help $id"
hoc_register_menu_data "Help" "Commands..." "Commands"\
	{ { summary "Tool for getting information on MGED's commands." }
          { see_also "?, help" } }
.$id.menubar.help add command -label "Apropos..." -underline 1\
	-command "ia_apropos .$id $screen"
hoc_register_menu_data "Help" "Apropos..." "Apropos"\
	{ { summary "Tool for searching for information about
MGED's commands." }
          { see_also "apropos" } }

if [info exists env(WEB_BROWSER)] {
    if [ file exists $env(WEB_BROWSER) ] {
	set web_cmd "exec $env(WEB_BROWSER) -display $screen $mged_html_dir/index.html &"
    }
}

# Minimal attempt to locate a browser
if ![info exists web_cmd] {
    if [ file exists /usr/X11/bin/netscape ] {
	set web_cmd "exec /usr/X11/bin/netscape -display $screen $mged_html_dir/index.html &"
    } elseif [ file exists /usr/X11/bin/Mosaic ] {
	set web_cmd "exec /usr/X11/bin/Mosaic -display $screen $mged_html_dir/index.html &"
    } else {
	set web_cmd "ia_man .$id $screen"
    }
}
.$id.menubar.help add command -label "Manual..." -underline 0 -command $web_cmd
hoc_register_menu_data "Help" "Manual..." "Manual"\
	{ { summary "Start a tool for browsing the online MGED manual.
The web browser that gets started is dependent, first, on the
WEB_BROWSER environment variable. If this variable exists and
the browser identified by this variable exists, then that browser
is used. Otherwise, the existence of /usr/X11/bin/netscape is
checked. Failing that, the existence of /usr/X11/bin/Mosaic is
checked. If a browser has still not been located, the built in
Tcl browser is used." } }

#==============================================================================
# PHASE 3: Bottom-row display
#==============================================================================

frame .$id.status
frame .$id.status.dpy
frame .$id.status.illum

label .$id.status.cent -textvar mged_display($mged_gui($id,active_dm),center) -anchor w
hoc_register_data .$id.status.cent "View Center"\
	{ { summary "These numbers indicate the view center in\nmodel coordinates (local units)." }
          { see_also "center, view, viewget, viewset" } }
label .$id.status.size -textvar mged_display($mged_gui($id,active_dm),size) -anchor w
hoc_register_data .$id.status.size "View Size"\
	{ { summary "This number indicates the view size (local units)." }
          { see_also size} }
label .$id.status.units -textvar mged_display(units) -anchor w -padx 4
hoc_register_data .$id.status.units "Units"\
	{ { summary "This indicates the local units.     " }
          { see_also units} }
label .$id.status.aet -textvar mged_display($mged_gui($id,active_dm),aet) -anchor w
hoc_register_data .$id.status.aet "View Orientation"\
	{ { summary "These numbers indicate the view orientation using azimuth,\nelevation and twist." }
        { see_also "ae, view, viewget, viewset" } }
label .$id.status.ang -textvar mged_display($mged_gui($id,active_dm),ang) -anchor w -padx 4
hoc_register_data .$id.status.ang "Rateknobs"\
	{ { summary "These numbers give some indication of\nrate of rotation about the x,y,z axes." }
        { see_also knob} }
label .$id.status.illum.label -textvar mged_gui($id,illum_label)
hoc_register_data .$id.status.illum.label "Status Area"\
	{ { summary "This area is for displaying either the frames per second,
the illuminated path, the keypoint during an edit
or the ADC attributes." } }

#==============================================================================
# PHASE 4: Text widget for interaction
#==============================================================================

frame .$id.tf
if {$comb} {
    text .$id.t -height $mged_gui($id,num_lines) -relief sunken -bd 2 -yscrollcommand ".$id.s set"
} else {
    text .$id.t -relief sunken -bd 2 -yscrollcommand ".$id.s set"
}
scrollbar .$id.s -relief flat -command ".$id.t yview"

bind .$id.t <Enter> "focus .$id.t; break"
hoc_register_data .$id.t "Command Window"\
	{ { summary "This is MGED's default command window. Its main
function is to allow the user to enter commands.
The command window supports command line editing
and command history. The two supported command line
edit modes are emacs and vi. Look under File/
Preferences/Command_Line_Edit to change the edit mode.

There are also two command interpretation modes. One
is where MGED performs object name matching (i.e. globbing
against the database) before passing the line to MGED's
built-in Tcl interpreter. This is the same behavior seen
in previous releases. The other command interpretation
mode (Tcl Evaluation) passes the command line directly to
the Tcl interpreter. Look under File/Preferences/
Special_Characters to change the interpetation mode.

The command window also supports cut and paste as well
as text scrolling. The default bindings are similar to
those found in typical X Window applications such as xterm.
For example:

ButtonPress-1           begin text selection
Button1-Motion         add to text selection
Shift-Button1            modify text selection
Double-Button-1        select word
Triple-Button-1         select line
ButtonPress-2           begin text operation
ButtonRelease-2        paste text
Button2-Motion         scroll text

Note - If motion was detected while Button2 was
being pressed, no text will be pasted. In this case,
it is assumed that scrolling was the intended operation.
The user can also scroll the window using the scrollbar." } }

set mged_gui($id,edit_style) $mged_default(edit_style)
set mged_gui(.$id.t,insert_char_flag) 0
set_text_key_bindings $id
set_text_button_bindings .$id.t

set mged_gui($id,cmd_prefix) ""
set mged_gui($id,more_default) ""
mged_print_prompt .$id.t "mged> "
.$id.t insert insert " "
beginning_of_line .$id.t
set mged_gui(.$id.t,moveView) 0
set mged_gui(.$id.t,freshline) 1
set mged_gui(.$id.t,scratchline) ""
set vi_state(.$id.t,overwrite_flag) 0
set vi_state(.$id.t,change_flag) 0
set vi_state(.$id.t,delete_flag) 0
set vi_state(.$id.t,search_flag) 0
set vi_state(.$id.t,search_char) ""
set vi_state(.$id.t,search_dir) ""

.$id.t tag configure sel -background #fefe8e
.$id.t tag configure result -foreground blue3
.$id.t tag configure oldcmd -foreground red3
.$id.t tag configure prompt -foreground red1

#==============================================================================
# Pack windows
#==============================================================================

setupmv $id
setmv $id

if { $comb } {
    if { $mged_gui($id,show_dm) } {
	grid $mged_gui($id,dmc) -sticky nsew -row 0 -column 0
    }
}

grid .$id.t .$id.s -in .$id.tf -sticky "nsew"
grid columnconfigure .$id.tf 0 -weight 1
grid rowconfigure .$id.tf 0 -weight 1

if { !$comb || ($comb && $mged_gui($id,show_cmd)) } {
    grid .$id.tf -sticky "nsew" -row 1 -column 0
}

grid .$id.status.cent .$id.status.size .$id.status.units .$id.status.aet\
	.$id.status.ang x -in .$id.status.dpy -sticky "ew"
grid columnconfigure .$id.status.dpy 5 -weight 1
grid .$id.status.dpy -sticky "ew"
grid .$id.status.illum.label x -sticky "ew"
grid columnconfigure .$id.status.illum 1 -weight 1
grid .$id.status.illum -sticky "w"
grid columnconfigure .$id.status 0 -weight 1

if { $mged_gui($id,show_status) } {
    grid .$id.status -sticky "ew" -row 2 -column 0
}

grid columnconfigure .$id 0 -weight 1
if { $comb } {
    # let only the display manager window grow
    grid rowconfigure .$id 0 -weight 1
} else {
    # let only the text window (i.e. enter commands here) grow
    grid rowconfigure .$id 1 -weight 1
}

#==============================================================================
# PHASE 5: Creation of other auxilary windows
#==============================================================================
mview_build_menubar $id

#==============================================================================
# PHASE 6: Further setup
#==============================================================================

if {[info procs cad_MenuFirstEntry] == ""} {
    # trigger the Tcl source file to be loaded
    cad_MenuFirstEntry ""
}

cmd_win open $id
_mged_tie $id $mged_gui($id,active_dm)
reconfig_gui_default $id

# Force display manager windows to update their respective color schemes
mged_apply_local $id "rset cs mode 0"
rset cs mode 1

if { $join_c } {
    collaborate join $id
}

trace variable mged_display($mged_gui($id,active_dm),fps) w "ia_changestate $id"
update_mged_vars $id
set mged_gui($id,qray_effects) [qray effects]

# reset current_cmd_list so that its cur_hist gets updated
cmd_win set $save_id

# cause all 4 windows to share menu state
share m $mged_gui($id,top).ul $mged_gui($id,top).ur
share m $mged_gui($id,top).ul $mged_gui($id,top).ll
share m $mged_gui($id,top).ul $mged_gui($id,top).lr

do_rebind_keys $id

# Throw away key events
bind $mged_gui($id,top) <KeyPress> { break }

set dbname [_mged_opendb]
set_wm_title $id $dbname

# set the size here in case the user didn't specify it in mged_default(ggeom)
set height [expr [winfo screenheight $mged_gui($id,top)] - 70]
set width $height
wm geometry $mged_gui($id,top) $width\x$height

# set geometry (i.e. size and position) according to mged_default(ggeom)
wm geometry $mged_gui($id,top) $mged_default(ggeom)

wm protocol $mged_gui($id,top) WM_DELETE_WINDOW "gui_destroy $id"

if { $comb } {
    if { !$mged_gui($id,show_dm) } {
	update
	set_dm_win $id
    }
} else {
    # Position command window in upper left corner
    wm geometry .$id $mged_default(geom)
    update

    # Prevent command window from resizing itself as labels change
    set geometry [wm geometry .$id]
    wm geometry .$id $geometry
}

set num_players [llength $mged_players]
switch $num_players {
    1 {
	.$id.menubar.file entryconfigure 11 -state disabled
    }
    2 {
	set id [lindex $mged_players 0]
	.$id.menubar.file entryconfigure 11 -state normal
    }
}
}

proc gui_destroy args {
    global mged_gui
    global mged_players
    global mged_collaborators

    if { [llength $args] != 1 } {
	return [help gui_destroy]
    }

    set id [lindex $args 0]

    set i [lsearch -exact $mged_players $id]
    if { $i == -1 } {
	return "gui_destroy: bad id - $id"
    }
    set mged_players [lreplace $mged_players $i $i]

    if { [lsearch -exact $mged_collaborators $id] != -1 } {
	collaborate quit $id
    }

    set mged_gui($id,multi_view) 0
    set mged_gui($id,show_edit_info) 0

    releasemv $id
    catch { cmd_win close $id }
    catch { destroy .mmenu$id }
    catch { destroy .sliders$id }
    catch { destroy $mged_gui($id,top) }
    catch { destroy .$id }

    if { [llength $mged_players] == 1 } {
	set id [lindex $mged_players 0]
	.$id.menubar.file entryconfigure 11 -state disabled
    }
}

proc reconfig_gui_default { id } {
    global mged_display

    cmd_win set $id
    set dm_id [_mged_tie $id]
    if { [llength $dm_id] != 1 } {
	return
    }

    .$id.status.cent configure -textvar mged_display($dm_id,center)
    .$id.status.size configure -textvar mged_display($dm_id,size)
    .$id.status.units configure -textvar mged_display(units)

    .$id.status.aet configure -textvar mged_display($dm_id,aet)
    .$id.status.ang configure -textvar mged_display($dm_id,ang)

    update_view_ring_entries $id s
    update_view_ring_entries $id d
#    reconfig_mmenu $id
}

proc reconfig_all_gui_default {} {
    global mged_collaborators

    foreach id $mged_collaborators {
	reconfig_gui_default $id
    }
}

proc update_mged_vars { id } {
    global mged_gui
    global forwarding_key
    global rateknobs
    global use_air
    global listen
    global fb
    global fb_all
    global fb_overlay
    global dlist
    global mouse_behavior
    global coords
    global rotate_about
    global transform
    global faceplate
    global perspective_mode
    global orig_gui

    winset $mged_gui($id,active_dm)
    set mged_gui($id,rateknobs) $rateknobs
    set mged_gui($id,adc_draw) [adc draw]
    set mged_gui($id,model_draw) [rset ax model_draw]
    set mged_gui($id,view_draw) [rset ax view_draw]
    set mged_gui($id,edit_draw) [rset ax edit_draw]
    set mged($id,use_air) $use_air
    set mged_gui($id,fb) $fb
    set mged_gui($id,fb_all) $fb_all
    set mged_gui($id,fb_overlay) $fb_overlay
    set mged_gui($id,dlist) $dlist
    set mged_gui($id,rubber_band) [rset rb draw]
    set mged_gui($id,mouse_behavior) $mouse_behavior
    set mged_gui($id,coords) $coords
    set mged_gui($id,rotate_about) $rotate_about
    set mged_gui($id,transform) $transform
    set mged_gui($id,grid_draw) [rset grid draw]
    set mged_gui($id,draw_snap) [rset grid snap]
    set mged_gui($id,faceplate) $faceplate
    set mged_gui($id,zclip) [dm set zclip]
    set mged_gui($id,perspective_mode) $perspective_mode
    set mged_gui($id,orig_gui) $orig_gui
    set mged_gui($id,forward_keys) $forwarding_key($mged_gui($id,active_dm))

    if {$mged_gui($id,dtype) == "ogl"} {
	set mged_gui($id,depthcue) [dm set depthcue]
	set mged_gui($id,zbuffer) [dm set zbuffer]
	set mged_gui($id,lighting) [dm set lighting]
    }

    if {$mged_gui($id,fb)} {
	.$id.menubar.settings.fb entryconfigure 7 -state normal
	set mged_gui($id,listen) $listen
    } else {
	.$id.menubar.settings.fb entryconfigure 7 -state disabled
	set mged_gui($id,listen) $listen
    }

    set_mged_v_axes_pos $id
}

proc set_mged_v_axes_pos { id } {
    global mged_gui

    set view_pos [rset ax view_pos]
    set hpos [lindex $view_pos 0]
    set vpos [lindex $view_pos 1]

    if {$hpos == 0 && $vpos == 0} {
	set mged_gui($id,view_pos) 0
    } elseif {$hpos < 0 && $vpos < 0} {
	set mged_gui($id,view_pos) 1
    } elseif {$hpos < 0 && $vpos > 0} {
	set mged_gui($id,view_pos) 2
    } elseif {$hpos > 0 && $vpos > 0} {
	set mged_gui($id,view_pos) 3
    } else {
	set mged_gui($id,view_pos) 4
    }
}

proc toggle_button_menu { id } {
    global mged_gui

    if [ winfo exists .mmenu$id ] {
	destroy .mmenu$id
	set mged_gui($id,classic_buttons) 0
	return
    }

    mmenu_init $id
}

proc toggle_edit_info { id } {
    global mged_gui
    global mged_display

    if {$mged_gui($id,show_edit_info)} {
	if {$mged_display(state) == "SOL EDIT"} {
	    init_edit_solid_int $id
	} elseif {$mged_display(state) == "OBJ EDIT"} {
	    build_edit_info $id
	}
    } else {
	if {$mged_display(state) == "SOL EDIT"} {
	    esolint_destroy $id
	} elseif {$mged_display(state) == "OBJ EDIT"} {
	    destroy_edit_info $id
	}
    }
}

proc build_edit_info { id } {
    global mged_gui

    if [winfo exists .sei$id] {
	raise .sei$id
	return
    }

    if {$mged_gui($id,show_edit_info)} {
	toplevel .sei$id -screen $mged_gui($id,screen)
	label .sei$id.l -bg black -fg yellow -textvar edit_info -font fixed
	pack .sei$id.l -expand 1 -fill both

	wm title .sei$id "$id\'s Edit Info"
	wm protocol .sei$id WM_DELETE_WINDOW "destroy_edit_info $id"
	wm geometry .sei$id $mged_gui($id,edit_info_pos)
    }
}

proc destroy_edit_info { id } {
    global mged_gui

    if ![winfo exists .sei$id] {
	return
    }

    regexp "\[-+\]\[0-9\]+\[-+\]\[0-9\]+" [wm geometry .sei$id] match
    set mged_gui($id,edit_info_pos) $match
    destroy .sei$id
}

# Print Mged Players
proc pmp {} {
    global mged_players

    return $mged_players
}

proc set_active_dm { id } {
    global mged_gui
    global mged_display
    global view_ring

    set new_dm $mged_gui($id,top).$mged_gui($id,dm_loc)

# Nothing to do
    if { $new_dm == $mged_gui($id,active_dm) } {
	return
    }

    trace vdelete mged_display($mged_gui($id,active_dm),fps) w "ia_changestate $id"

    # make inactive
    winset $mged_gui($id,active_dm)
    rset cs mode 0

    set mged_gui($id,active_dm) $new_dm

    # make active
    winset $mged_gui($id,active_dm)
    rset cs mode 1

    trace variable mged_display($mged_gui($id,active_dm),fps) w "ia_changestate $id"

    update_mged_vars $id
    set view_ring($id) [view_ring get]

    _mged_tie $id $mged_gui($id,active_dm)
    reconfig_gui_default $id

    if {!$mged_gui($id,multi_view)} {
	setmv $id
    }

    # update view_ring entries
    update_view_ring_entries $id s
    update_view_ring_entries $id d

    # update adc control panel
    adc_load $id

    # update grid control panel
    grid_control_reset $id

    # update query ray control panel
    qray_reset $id

    # update raytrace control panel
    update_Raytrace $id

    set dbname [_mged_opendb]
    set_wm_title $id $dbname
}

proc set_wm_title { id dbname } {
    global mged_gui

    if {$mged_gui($id,top) == $mged_gui($id,dmc)} {
	if {$mged_gui($id,dm_loc) == "ul"} {
	    wm title $mged_gui($id,top) "MGED 5.0 Graphics Window ($id) - $dbname - Upper Left"
	    wm title .$id "MGED 5.0 Command Window ($id) - $dbname - Upper Left"
	} elseif {$mged_gui($id,dm_loc) == "ur"} {
	    wm title $mged_gui($id,top) "MGED 5.0 Graphics Window ($id) - $dbname - Upper Right"
	    wm title .$id "MGED 5.0 Command Window ($id) - $dbname - Upper Right"
	} elseif {$mged_gui($id,dm_loc) == "ll"} {
	    wm title $mged_gui($id,top) "MGED 5.0 Graphics Window ($id) - $dbname - Lower Left"
	    wm title .$id "MGED 5.0 Command Window ($id) - $dbname - Lower Left"
	} elseif {$mged_gui($id,dm_loc) == "lr"} {
	    wm title $mged_gui($id,top) "MGED 5.0 Graphics Window ($id) - $dbname - Lower Right"
	    wm title .$id "MGED 5.0 Command Window ($id) - $dbname - Lower Right"
	}
    } else {
	if {$mged_gui($id,dm_loc) == "ul"} {
	    wm title $mged_gui($id,top) "MGED 5.0 Command Window ($id) - $dbname - Upper Left"
	} elseif {$mged_gui($id,dm_loc) == "ur"} {
	    wm title $mged_gui($id,top) "MGED 5.0 Command Window ($id) - $dbname - Upper Right"
	} elseif {$mged_gui($id,dm_loc) == "ll"} {
	    wm title $mged_gui($id,top) "MGED 5.0 Command Window ($id) - $dbname - Lower Left"
	} elseif {$mged_gui($id,dm_loc) == "lr"} {
	    wm title $mged_gui($id,top) "MGED 5.0 Command Window ($id) - $dbname - Lower Right"
	}
    }
}

proc set_cmd_win { id } {
    global mged_gui

    if { $mged_gui($id,show_cmd) } {
	if { $mged_gui($id,show_dm) } {
	    .$id.t configure -height $mged_gui($id,num_lines)
	    grid .$id.tf -sticky nsew -row 1 -column 0
	    #		update
	    #		setmv $id
	} else {
	    grid .$id.tf -sticky nsew -row 0 -column 0
	}
    } else {
	grid forget .$id.tf

	if { !$mged_gui($id,show_dm) } {
	    set mged_gui($id,show_dm) 1
	    grid $mged_gui($id,dmc) -sticky nsew -row 0 -column 0
	    update
	    setmv $id
	}
    }
}

proc set_dm_win { id } {
    global mged_gui

    if { $mged_gui($id,show_dm) } {
	if { $mged_gui($id,show_cmd) } {
	    grid forget .$id.tf
	    .$id.t configure -height $mged_gui($id,num_lines)
	    update
	}

	grid $mged_gui($id,dmc) -sticky nsew -row 0 -column 0

	if { $mged_gui($id,show_cmd) } {
	    grid .$id.tf -sticky nsew -row 1 -column 0
	    update
	    .$id.t see end
	}

	setmv $id
    } else {
	grid forget $mged_gui($id,dmc)
	
	set mged_gui($id,show_cmd) 1
	set_cmd_win $id
	set fh [get_font_height .$id.t]
	set h [winfo height $mged_gui($id,top)]

	if { $mged_gui($id,show_status) } {
	    set h [expr $h - [winfo height .$id.status]]
	}

	set nlines [expr $h / $fh]
	.$id.t configure -height $nlines
    }
}

proc view_ring_add { id } {
    global mged_gui
    global view_ring
    global mged_collaborators

#    if {$mged_gui($id,dm_loc) != "lv"} {
#	winset $mged_gui($id,active_dm)
#    }
    winset $mged_gui($id,active_dm)

    _mged_view_ring add

    set i [lsearch -exact $mged_collaborators $id]
    if {$i != -1 && "$mged_gui($id,top).ur" == "$mged_gui($id,active_dm)"} {
	foreach cid $mged_collaborators {
	    if {"$mged_gui($cid,top).ur" == "$mged_gui($cid,active_dm)"} {
		update_view_ring_entries $cid s
		update_view_ring_entries $cid d
		winset $mged_gui($cid,active_dm)
		set view_ring($cid) [view_ring get]
	    }
	}
    } else {
	update_view_ring_entries $id s
	update_view_ring_entries $id d
	set view_ring($id) [view_ring get]
    }
}

proc view_ring_delete { id vid } {
    global mged_gui
    global view_ring
    global mged_collaborators

#    if {$mged_gui($id,dm_loc) != "lv"} {
#	winset $mged_gui($id,active_dm)
#    }
    winset $mged_gui($id,active_dm)

    _mged_view_ring delete $vid

    set i [lsearch -exact $mged_collaborators $id]
    if { $i != -1 && "$mged_gui($id,top).ur" == "$mged_gui($id,active_dm)"} {
	foreach cid $mged_collaborators {
	    if {"$mged_gui($cid,top).ur" == "$mged_gui($cid,active_dm)"} {
		update_view_ring_entries $cid s
		update_view_ring_entries $cid d
		winset $mged_gui($cid,active_dm)
		set view_ring($cid) [view_ring get]
	    }
	}
    } else {
	update_view_ring_entries $id s
	update_view_ring_entries $id d
	set view_ring($id) [view_ring get]
    }
}

proc view_ring_goto { id vid } {
    global mged_gui
    global view_ring
    global mged_collaborators

#    if {$mged_gui($id,dm_loc) != "lv"} {
#	winset $mged_gui($id,active_dm)
#    }
    winset $mged_gui($id,active_dm)

    _mged_view_ring goto $vid

    set i [lsearch -exact $mged_collaborators $id]
    if { $i != -1 && "$mged_gui($id,top).ur" == "$mged_gui($id,active_dm)"} {
	foreach cid $mged_collaborators {
	    if {"$mged_gui($cid,top).ur" == "$mged_gui($cid,active_dm)"} {
		set view_ring($cid) $vid
	    }
	}
    } else {
	set view_ring($id) $vid
    }
}

proc view_ring_next { id } {
    global mged_gui
    global view_ring
    global mged_collaborators

#    if {$mged_gui($id,dm_loc) != "lv"} {
#	winset $mged_gui($id,active_dm)
#    }
    winset $mged_gui($id,active_dm)

    _mged_view_ring next

    set i [lsearch -exact $mged_collaborators $id]
    if { $i != -1 && "$mged_gui($id,top).ur" == "$mged_gui($id,active_dm)"} {
	foreach cid $mged_collaborators {
	    if {"$mged_gui($cid,top).ur" == "$mged_gui($cid,active_dm)"} {
		winset $mged_gui($cid,active_dm)
		set view_ring($cid) [view_ring get]
	    }
	}
    } else {
	set view_ring($id) [view_ring get]
    }
}

proc view_ring_prev { id } {
    global mged_gui
    global view_ring
    global mged_collaborators

#    if {$mged_gui($id,dm_loc) != "lv"} {
#	winset $mged_gui($id,active_dm)
#    }
    winset $mged_gui($id,active_dm)

    _mged_view_ring prev

    set i [lsearch -exact $mged_collaborators $id]
    if { $i != -1 && "$mged_gui($id,top).ur" == "$mged_gui($id,active_dm)"} {
	foreach cid $mged_collaborators {
	    if {"$mged_gui($cid,top).ur" == "$mged_gui($cid,active_dm)"} {
		winset $mged_gui($cid,active_dm)
		set view_ring($cid) [view_ring get]
	    }
	}
    } else {
	set view_ring($id) [view_ring get]
    }
}

proc view_ring_toggle { id } {
    global mged_gui
    global view_ring
    global mged_collaborators

#    if {$mged_gui($id,dm_loc) != "lv"} {
#	winset $mged_gui($id,active_dm)
#    }
    winset $mged_gui($id,active_dm)

    _mged_view_ring toggle

    set i [lsearch -exact $mged_collaborators $id]
    if { $i != -1 && "$mged_gui($id,top).ur" == "$mged_gui($id,active_dm)"} {
	foreach cid $mged_collaborators {
	    if {"$mged_gui($cid,top).ur" == "$mged_gui($cid,active_dm)"} {
		winset $mged_gui($cid,active_dm)
		set view_ring($cid) [view_ring get]
	    }
	}
    } else {
	set view_ring($id) [view_ring get]
    }
}

proc update_view_ring_entries { id m } {
    global view_ring

    set views [view_ring get -a]
    set llen [llength $views]

    if {$m == "s"} {
	set w .$id.menubar.viewring.select
	$w delete 0 end
	for {set i 0} {$i < $llen} {incr i} {
	    $w add radiobutton -value [lindex $views $i] -variable view_ring($id)\
		    -label [lindex $views $i] -command "view_ring_goto $id [lindex $views $i]"
	}
    } elseif {$m == "d"} {
	set w .$id.menubar.viewring.delete
	$w delete 0 end
	for {set i 0} {$i < $llen} {incr i} {
	    $w add command -label [lindex $views $i]\
		    -command "view_ring_delete $id [lindex $views $i]"
	}
    } else {
	puts "Usage: update_view_ring_entries w s|d"
    }
}

proc update_view_ring_labels { id } {
    global mged_gui
    global view_ring

    winset $mged_gui($id,active_dm)
    set view_ring($id) [view_ring get]
    set views [view_ring get -a]
    set llen [llength $views]

    set ws .$id.menubar.viewring.select
    set wd .$id.menubar.viewring.delete
    for {set i 0} {$i < $llen} {incr i} {
	view_ring goto [lindex $views $i]
	set aet [view aet]
	set aet [format "az=%.2f el=%.2f tw=%.2f"\
		[lindex $aet 0] [lindex $aet 1] [lindex $aet 2]]
	set center [view center]
	set center [format "cent=(%.3f %.3f %.3f)"\
		[lindex $center 0] [lindex $center 1] [lindex $center 2]]
	set size [format "size=%.3f" [view size]]
	$ws entryconfigure $i -label "$center $size $aet"
	$wd entryconfigure $i -label "$center $size $aet"
    }

    view_ring goto $view_ring($id)
}

proc toggle_status_bar { id } {
    global mged_gui

    if {$mged_gui($id,show_status)} {
	grid .$id.status -sticky ew -row 2 -column 0
    } else {
	grid forget .$id.status
    }
}

proc set_transform { id } {
    global mged_gui
    global transform

    winset $mged_gui($id,top).ul
    set transform $mged_gui($id,transform)
    default_mouse_bindings $mged_gui($id,top).ul

    winset $mged_gui($id,top).ur
    set transform $mged_gui($id,transform)
    default_mouse_bindings $mged_gui($id,top).ur

    winset $mged_gui($id,top).ll
    set transform $mged_gui($id,transform)
    default_mouse_bindings $mged_gui($id,top).ll

    winset $mged_gui($id,top).lr
    set transform $mged_gui($id,transform)
    default_mouse_bindings $mged_gui($id,top).lr

    winset $mged_gui($id,active_dm)
}

proc do_rebind_keys { id } {
    global mged_gui

    bind $mged_gui($id,top).ul <Control-n> "winset $mged_gui($id,top).ul; view_ring_next $id; break" 
    bind $mged_gui($id,top).ur <Control-n> "winset $mged_gui($id,top).ur; view_ring_next $id; break" 
    bind $mged_gui($id,top).ll <Control-n> "winset $mged_gui($id,top).ll; view_ring_next $id; break" 
    bind $mged_gui($id,top).lr <Control-n> "winset $mged_gui($id,top).lr; view_ring_next $id; break" 

    bind $mged_gui($id,top).ul <Control-p> "winset $mged_gui($id,top).ul; view_ring_prev $id; break" 
    bind $mged_gui($id,top).ur <Control-p> "winset $mged_gui($id,top).ur; view_ring_prev $id; break" 
    bind $mged_gui($id,top).ll <Control-p> "winset $mged_gui($id,top).ll; view_ring_prev $id; break" 
    bind $mged_gui($id,top).lr <Control-p> "winset $mged_gui($id,top).lr; view_ring_prev $id; break" 

    bind $mged_gui($id,top).ul <Control-t> "winset $mged_gui($id,top).ul; view_ring_toggle $id; break" 
    bind $mged_gui($id,top).ur <Control-t> "winset $mged_gui($id,top).ur; view_ring_toggle $id; break" 
    bind $mged_gui($id,top).ll <Control-t> "winset $mged_gui($id,top).ll; view_ring_toggle $id; break" 
    bind $mged_gui($id,top).lr <Control-t> "winset $mged_gui($id,top).lr; view_ring_toggle $id; break" 
}

proc adc { args } {
    global mged_gui
    global transform

    set result [eval _mged_adc $args]

    # toggling ADC on/off
    if { ![llength $args] } {
	set dm_id [winset]
	set tie_list [_mged_tie]
	set id mged

	# see if dm_id is tied to a command window
	foreach pair $tie_list {
	    if { [lindex $pair 1] == $dm_id } {
		set id [lindex $pair 0]
		break
	    }
	}

	if {[info exists mged_gui($id,adc_draw)]} {
	    set mged_gui($id,adc_draw) [_mged_adc draw]
	}

	default_mouse_bindings [winset]
    }

    return $result
}

proc set_listen { id } {
    global mged_gui
    global listen

    mged_apply $id "set listen \$mged_gui($id,listen)"

# In case things didn't work.
    set mged_gui($id,listen) $listen
}

proc set_fb { id } {
    global mged_gui
    global listen

    mged_apply $id "set fb \$mged_gui($id,fb)"

    if {$mged_gui($id,fb)} {
	.$id.menubar.settings.fb entryconfigure 7 -state normal
	.$id.menubar.modes entryconfigure 4 -state normal
	set mged_gui($id,listen) 1
	mged_apply $id "set listen \$mged_gui($id,listen)"
    } else {
	.$id.menubar.settings.fb entryconfigure 7 -state disabled
	.$id.menubar.modes entryconfigure 4 -state disabled
	set mged_gui($id,listen) 0
    }
}

proc get_font_height { w } {
    if { [winfo class $w] != "Text" } {
	return 0
    }

    set tmp_font [lindex [$w configure -font] 4]

    return [font metrics $tmp_font -linespace]
}

proc get_cmd_win_height { id } {
    global mged_gui

    set fh [get_font_height .$id.t]

    return [expr $fh * $mged_gui($id,num_lines)]
}
