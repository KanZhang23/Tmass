
# There is no way to find out what the user did after sourcing this file,
# so it is impossible to figure out the required cleanup. Just tell the
# user to restart tclsh/wish if this file is sourced more than once.
set source_dir [file dirname [info script]]
source $source_dir/example_utils.tcl
check_not_loaded 

# Initialize Histo-Scope
histo_init
hs::config_histoscope -titleFont {Helvetica 32} -bufferGraphics 1

# Create a dummy histogram which will be used as a drawing board
set nbins 50
set id [random_histo gauss $nbins 0 $nbins [expr $nbins/2] 8 1000]
hs::show_histogram $id -window winlatex -geometry 600x530 \
        -font {Helvetica 20} -title {LaTeX Comments} \
        -fill solid -fillcolor firebrick

# Write down couple formulas
hs::latex -ipadx {3 3} -ipady {3 2} -anchor nw -scale 2 {
    $$ \textcolor{red}{\int \delta(q^{2}-m^{2}) d^{4}q = 
    \frac{d\,^{3}\,\bf q}{2 \sqrt{|{\bf q}|^{2}+m^{2}}}} $$
} {0.1 0.9}

hs::latex -ipadx {3 3} -ipady {3 3} -anchor nw -scale 1.5 {
    \textcolor{blue}{\begin{center} Bethe-Bloch formula: \end{center}
    $$ -\frac{dE}{dx}=\frac{4\pi n_{a} z^{2} e^{4}}{m_{e}\/\beta^{2} c^{2}}Z
    \left[\log\frac{2\/m_{e}\/\beta^{2} c^{2}}
    {(1-\beta^{2})I} - \beta^{2} - \frac{\delta}{2}\right] $$}
} {0.2 0.7}

# Create a fancy table
hs::latex -anchor nw -scale 1.5 -border off -background yellow {
\begin{tabular}{|c|c|c|} \hline
Variable & \raisebox{0ex}[2.5ex][1ex]{Density} & Interval \\ \hline\
$k^{2}$  &  \raisebox{0ex}[5.5ex][2.5ex]{${\displaystyle 
\frac{\sqrt{1-\frac{4m_{e}^{2}}{k^{2}}}}{k^{2}}}$}
& $(4m_{e}^{2},(m_{\tau}-m_{\mu})^{2})$ \\
$q^{2}$ & \raisebox{0ex}[4ex][3ex]{${\displaystyle 
\frac{\lambda(q^{2},m_{\mu}^{2},k^{2})}{(q^{2}-m_{\mu}^{2})^{2}}}$}
& $\left( (m_{\mu}+\sqrt{k^{2}})^{2},m_{\tau}^{2}) \right)$ \\
$r^{2}$  & \raisebox{0ex}[2ex][2ex]{$\lambda(m_{\tau}^{2},r^{2},q^{2})$}
& $\left(0, (m_{\tau}-\sqrt{q^{2}})^{2}\right)$\\
\hline
\end{tabular}
} {0.3 0.49}
