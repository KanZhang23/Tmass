proc next_uid {} {
    global strd_next_uid
    if {![info exists strd_next_uid]} {
	set strd_next_uid 0
    }
    incr strd_next_uid
}

proc parse_strd_header {lines} {
    if {[llength $lines] != 7} {
	error "Bad StRD header size"
    }
    foreach {top dname empty fileformat start_values cert_value data} $lines {}
    if {![string equal $top "NIST/ITL StRD"]} {
	error "Expected \"NIST/ITL StRD\", got \"$top\""
    }
    if {![string equal $fileformat "File Format:   ASCII"]} {
	error "Expected \"File Format:   ASCII\", got \"$fileformat\""
    }
    if {![regexp {Starting Values\s+\(lines\s+(\d+)\s+to\s+(\d+)\)} \
	    $start_values dumm start_param end_param]} {
	error "Failed to parse the line \"$start_values\""
    }
    if {![regexp {Data\s+\(lines\s+(\d+)\s+to\s+(\d+)\)} \
	    $data dumm start_data end_data]} {
	error "Failed to parse the line \"$data\""
    }
    # Renumber the lines from 0
    incr start_param -1
    incr end_param -1
    incr start_data -1
    incr end_data -1
    list $start_param $end_param $start_data $end_data
}

proc at_max_precision {script} {
    global tcl_precision errorInfo
    set old_precision $tcl_precision
    set tcl_precision 17
    if {[catch {uplevel $script} result]} {
        set savedInfo $errorInfo
        set tcl_precision $old_precision
        error $result $savedInfo
    }
    set tcl_precision $old_precision
    set result
}

proc retrieve_strd_dataset {name filename} {
    package require http
    set basic_url "http://www.itl.nist.gov/div898/strd/nls/data/LINKS/DATA/"
    set file $name.dat
    set url "${basic_url}$file"
    set token [::http::geturl $url]
    set status [::http::status $token]
    set data [::http::data $token]
    ::http::cleanup $token
    if {![string equal $status "ok"]} {
	error "Failed to fetch $url"
    }
    puts "Retrieved $name data from $url"
    file mkdir [file dirname $filename]
    set chan [open $filename w]
    puts -nonewline $chan $data
    close $chan
    return
}

proc strd_dataset_list {} {
    list Bennett5 BoxBOD Chwirut1 Chwirut2 DanWood DanielWood ENSO Eckerle4 \
	    Gauss1 Gauss2 Gauss3 Hahn1 Kirby2 Lanczos1 Lanczos2 Lanczos3 \
	    MGH09 MGH10 MGH17 Misra1a Misra1b Misra1c Misra1d Nelson Rat42 \
	    Rat43 Roszman1 Thurber
}

proc add_strd_model {fit dataset_name} {
    switch $dataset_name {
	Bennett5 {
	    hs::function_compile model "$dataset_name fit" \
		    {b1 * pow((b2+x),(-1.0/b3))} {b1 b2 b3}
	    $fit function add model -mapping identical
	}
	BoxBOD {
	    $fit function add exp_rise -mapping {
		H_l = 0.0;
		x0 = 0.0;
		H_r = %b1;
		width = 1.0/%b2;
	    }
	}
	Chwirut1 -
	Chwirut2 {
	    hs::function_compile model "$dataset_name fit" \
		    {exp(-b1*x)/(b2+b3*x)} {b1 b2 b3}
	    $fit function add model -mapping identical
	}
	DanWood -
	DanielWood {
	    hs::function_compile model "$dataset_name fit" \
		    {b1*pow(x,b2)} {b1 b2}
	    $fit function add model -mapping identical
	}
	ENSO {
	    set pi [expr {atan2(0,-1)}]
	    at_max_precision {
		hs::function_compile model "$dataset_name fit" [subst {
			b1 + b2*cos( 2*$pi*x/12 ) + b3*sin( 2*$pi*x/12 )
			+ b5*cos( 2*$pi*x/b4 ) + b6*sin( 2*$pi*x/b4 )
			+ b8*cos( 2*$pi*x/b7 ) + b9*sin( 2*$pi*x/b7 )
		}] {b1 b2 b3 b4 b5 b6 b7 b8 b9}
	    }
	    $fit function add model -mapping identical
	}
	Eckerle4 {
	    set sqrt2pi [expr {sqrt(2.0*atan2(0,-1))}]
	    at_max_precision {
		$fit function add gauss -mapping [subst {
		    area = %b1 * $sqrt2pi;
		    mean = %b3;
		    sigma = %b2;
		}]
	    }
	}
	Gauss1 -
	Gauss2 -
	Gauss3 {
	    hs::function gauss copy gauss1
	    set sqrtpi [expr {sqrt(atan2(0,-1))}]
	    set sqrt2 [expr {sqrt(2.0)}]
	    at_max_precision {
		$fit function add exp_pdf -mapping {
		    x0 = 0.0;
		    width = 1.0/%b2;
		    area = %b1/%b2;
		}
		$fit function add gauss -mapping [subst {
		    sigma = %b5/$sqrt2;
		    mean = %b4;
		    area = $sqrtpi*%b3*%b5;
		}]
		$fit function add gauss1 -mapping [subst {
		    sigma = %b8/$sqrt2;
		    mean = %b7;
		    area = $sqrtpi*%b6*%b8;
		}]
	    }
	}
	Kirby2 {
	    hs::function_compile model "$dataset_name fit" \
		    {(b1 + b2*x + b3*x*x) / (1.0 + b4*x + b5*x*x)} \
		    {b1 b2 b3 b4 b5}
	    $fit function add model -mapping identical
	}
	Lanczos1 -
	Lanczos2 -
	Lanczos3 {
	    hs::function_compile model "$dataset_name fit" \
		    {b1*exp(-b2*x) + b3*exp(-b4*x) + b5*exp(-b6*x)} \
		    {b1 b2 b3 b4 b5 b6}
	    $fit function add model -mapping identical
	}
	MGH09 {
	    hs::function_compile model "$dataset_name fit" \
		    {b1*(x*x+x*b2) / (x*x+x*b3+b4)} {b1 b2 b3 b4}
	    $fit function add model -mapping identical
	}
	MGH10 {
	    hs::function_compile model "$dataset_name fit" \
		    {b1 * exp(b2/(x+b3))} {b1 b2 b3}
	    $fit function add model -mapping identical
	}
	MGH17 {
	    hs::function_compile model "$dataset_name fit" \
		    {b1 + b2*exp(-x*b4) + b3*exp(-x*b5)} {b1 b2 b3 b4 b5}
	    $fit function add model -mapping identical
	}
	Misra1a {
	    $fit function add exp_rise -mapping {
		H_l = 0.0;
		x0 = 0.0;
		H_r = %b1;
		width = 1.0/%b2;
	    }
	}
	Misra1b {
	    hs::function_compile model "$dataset_name fit" \
		    {b1 * (1.0-pow(1.0+b2*x/2.0,-2.0))} {b1 b2}
	    $fit function add model -mapping identical
	}
	Misra1c {
	    hs::function_compile model "$dataset_name fit" \
		    {b1 * (1.0-1.0/sqrt(1.0+2.0*b2*x))} {b1 b2}
	    $fit function add model -mapping identical
	}
	Misra1d {
	    hs::function_compile model "$dataset_name fit" \
		    {b1*b2*x/(1.0+b2*x)} {b1 b2}
	    $fit function add model -mapping identical
	}
	Nelson {
	    hs::function_compile model "$dataset_name fit" \
		    {b1 - b2*x * exp(-b3*y)} {b1 b2 b3}
	    $fit function add model -mapping identical
	}
	Rat42 {
	    hs::function_compile model "$dataset_name fit" \
		    {b1 / (1.0+exp(b2-b3*x))} {b1 b2 b3}
	    $fit function add model -mapping identical
	}
	Rat43 {
	    hs::function_compile model "$dataset_name fit" \
		    {b1 / (pow(1.0+exp(b2-b3*x),1.0/b4))} {b1 b2 b3 b4}
	    $fit function add model -mapping identical
	}
	Roszman1 {
	    set pi [expr {atan2(0,-1)}]
	    at_max_precision {
		hs::function_compile model "$dataset_name fit" \
			[subst {b1 - b2*x - atan(b3/(x-b4))/$pi}] {b1 b2 b3 b4}
	    }
	    $fit function add model -mapping identical
	}
	Hahn1 -
	Thurber {
	    hs::function_compile model "$dataset_name fit" \
		    {(b1+b2*x+b3*x*x+b4*x*x*x)/(1.0+b5*x+b6*x*x+b7*x*x*x)} \
		    {b1 b2 b3 b4 b5 b6 b7}
	    $fit function add model -mapping identical
	}
	default {
	    error "Unknown StRD dataset name \"$dataset_name\""
	}
    }
    return
}
