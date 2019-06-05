
# There is no way to find out what the user did after sourcing this file,
# so it is impossible to figure out the required cleanup. Just tell the
# user to restart tclsh/wish if this file is sourced more than once.
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded 

# Initialize Histo-Scope
histo_init
hs::config_histoscope -printTitles off

# Create a dummy histogram which will be used as a drawing board
set id [hs::create_1d_hist [next_uid] "Latex Colors" "Tmp" X Y 10 0 1]
hs::fill_1d_hist $id 0.5 0
hs::show_histogram $id -window winlatex -geometry 600x490 -ipady {100 100}

# Form the latex code
set latex {
    \begin{center}\large
    The 68 standard colors known to dvips\vspace{0.5ex}
    \begin{tabular}{|l|l|l|l|}
    \hline
}
set i 0
foreach color {
    Apricot Aquamarine Bittersweet Black Blue BlueGreen BlueViolet BrickRed
    Brown BurntOrange CadetBlue CarnationPink Cerulean CornflowerBlue Cyan
    Dandelion DarkOrchid Emerald ForestGreen Fuchsia Goldenrod Gray Green
    GreenYellow JungleGreen Lavender LimeGreen Magenta Mahogany Maroon Melon
    MidnightBlue Mulberry NavyBlue OliveGreen Orange OrangeRed Orchid Peach
    Periwinkle PineGreen Plum ProcessBlue Purple RawSienna Red RedOrange
    RedViolet Rhodamine RoyalBlue RoyalPurple RubineRed Salmon SeaGreen Sepia
    SkyBlue SpringGreen Tan TealBlue Thistle Turquoise Violet VioletRed
    White WildStrawberry Yellow YellowGreen YellowOrange
} {
    append latex [subst -nocommands -nobackslash \
	    {\textcolor[named]{$color}{$color}}]
    if {[expr [incr i] % 4] == 0} {
	append latex {\\ \hline} "\n"
    } else {
	append latex & "\n"
    }
}
append latex {
    \end{tabular}
    \end{center}
}

# Put the table on top of the plot
hs::latex -coords winabs -anchor sw -ipadx {6 5} -ipady {5 5} \
        -scale 1.37 $latex {10 10} -border off
