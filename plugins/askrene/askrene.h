#ifndef LIGHTNING_PLUGINS_ASKRENE_ASKRENE_H
#define LIGHTNING_PLUGINS_ASKRENE_ASKRENE_H
#include "config.h"
#include <bitcoin/short_channel_id.h>
#include <ccan/list/list.h>
#include <common/amount.h>

/* We reserve a path being used.  This records how many and how much */
struct reserve {
	size_t num_htlcs;
	struct short_channel_id_dir sciddir;
	struct amount_msat amount;
};

/* A single route. */
struct route {
	/* Actual path to take */
	struct route_hop *hops;
	/* Probability estimate (0-1) */
	double success_prob;
};

/* Grab-bag of "globals" for this plugin */
struct askrene {
	struct plugin *plugin;
	struct gossmap *gossmap;
	/* List of layers */
	struct list_head layers;
};

#endif /* LIGHTNING_PLUGINS_ASKRENE_ASKRENE_H */