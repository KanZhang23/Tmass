#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "fit_config.h"
#include "minuit.h"

#define checkfree(pointer) do {if (pointer) {free(pointer); pointer=0;}} while(0);

static Fit_config *current_fit_config = NULL;
static char *current_fit_name = NULL;

Fit_config * hs_current_fit_conf(void)
{
    return current_fit_config;
}

int hs_set_current_fit(const char *name)
{
    Fit_config *config;
    char *empty = "";
    int set, title_set = 0;

    if (name)
    {
	if (name[0] == '\0')
	    return hs_set_current_fit(NULL);
	if (current_fit_name)
	    if (strcmp(name, current_fit_name) == 0)
		return 0;
	config = hs_find_fit_byname(name);
	if (config == NULL)
	    return 1;

	/* At this point we know that the config pointer will change */
	assert(current_fit_config != config);
	if (current_fit_config)
	{
	    current_fit_config->par_synched = 0;
	    current_fit_config->options_synched = 0;
	    hs_declare_fit_complete(current_fit_config, 0);
	    /* If possible, free up some storage taken by cached data */
	    for (set=0; set<current_fit_config->nsets; ++set)
		if (current_fit_config->fitsets[set])
		{
		    checkfree(current_fit_config->fitsets[set]->iweights);
		    current_fit_config->fitsets[set]->nweights = 0;
		    checkfree(current_fit_config->fitsets[set]->points);
		    current_fit_config->fitsets[set]->npoints = 0;
		}
	}
	checkfree(current_fit_name);
	current_fit_name = strdup(name);
	if (current_fit_name == NULL)
	{
	    current_fit_config = NULL;
	    return 2;
	}
	current_fit_config = config;
	config->par_synched = 0;
	config->options_synched = 0;
	hs_declare_fit_complete(config, 0);

	/* Set up Minuit job title */
	if (config->title)
	    if (config->title[0])
	    {
		hs_minuit_set_title(config->title);
		title_set = 1;
	    }
	if (!title_set)
	    hs_minuit_set_title(hs_default_fit_title(name));
    }
    else
    {
	if (current_fit_config)
	    hs_declare_fit_complete(current_fit_config, 0);
	current_fit_config = NULL;
	checkfree(current_fit_name);
	hs_minuit_set_title(empty);
    }
    return 0;
}

const char * hs_current_fit_name(void)
{
    if (current_fit_config)
	return current_fit_name;
    else
	return NULL;
}

int hs_reassign_current_fit_name(const char *name)
{
    char *c;

    assert(name);
    assert(name[0] != '\0');
    c = strdup(name);
    if (c == NULL)
	return 1;
    checkfree(current_fit_name);
    current_fit_name = c;
    return 0;
}
