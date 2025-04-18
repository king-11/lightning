#ifndef LIGHTNING_PLUGINS_CHANNEL_HINT_H
#define LIGHTNING_PLUGINS_CHANNEL_HINT_H

#include "config.h"
#include <bitcoin/short_channel_id.h>
#include <ccan/short_types/short_types.h>
#include <ccan/time/time.h>
#include <common/amount.h>
#include <common/json_stream.h>
#include <plugins/libplugin.h>

/* Information about channels we inferred from a) looking at our channels, and
 * b) from failures encountered during attempts to perform a payment. These
 * are attached to the root payment, since that information is
 * global. Attempts update the estimated channel capacities when starting, and
 * get remove on failure. Success keeps the capacities, since the capacities
 * changed due to the successful HTLCs. */
struct channel_hint {
	/* The timestamp this observation was made. Used to let the
	 * constraint expressed by this hint decay over time, until it
	 * is fully relaxed, at which point we can forget about it
	 * (the structural information is the best we can do in that
	 * case).
	 */
	u32 timestamp;
	/* The short_channel_id we're going to use when referring to
	 * this channel. This can either be the real scid, or the
	 * local alias. The `pay` algorithm doesn't really care which
	 * one it is, but we'll prefer the scid as that's likely more
	 * readable than the alias. */
	struct short_channel_id_dir scid;

	/* Upper bound on remove channels inferred from payment failures. */
	struct amount_msat estimated_capacity;

	/* Is the channel enabled? */
	bool enabled;

	/* Non-null if we are one endpoint of this channel */
	struct local_hint *local;

	/* The total `amount_msat` that were used to fund the
	 * channel. This is always smaller gte the estimated_capacity
	 * (after normalization) */
	struct amount_msat capacity;
};

size_t channel_hint_hash(const struct short_channel_id_dir *out);

const struct short_channel_id_dir *channel_hint_keyof(const struct channel_hint *out);

bool channel_hint_eq(const struct channel_hint *a,
		     const struct short_channel_id_dir *b);

HTABLE_DEFINE_NODUPS_TYPE(struct channel_hint, channel_hint_keyof,
			  channel_hint_hash, channel_hint_eq, channel_hint_map)

/* A collection of channel_hint instances, allowing us to handle and
 * update them more easily. */
struct channel_hint_set {
	/* htable of channel_hints, indexed by scid and direction. */
	struct channel_hint_map *hints;
};

bool channel_hint_update(const struct timeabs now,
			 struct channel_hint *hint);

void channel_hint_to_json(const char *name, const struct channel_hint *hint,
			  struct json_stream *dest);

struct channel_hint *channel_hint_from_json(const tal_t *ctx,
					    const char *buffer,
					    const jsmntok_t *toks);

struct channel_hint_set *channel_hint_set_new(const tal_t *ctx);

/* Relax all channel_hints in this set, based on the time that has elapsed. */
void channel_hint_set_update(struct channel_hint_set *set, const struct timeabs now);

/**
 * Look up a `channel_hint` from a `channel_hint_set` for a scidd.
 */
struct channel_hint *channel_hint_set_find(const struct channel_hint_set *self,
					   const struct short_channel_id_dir *scidd);

/**
 * Add a new observation to the `channel_hint_set`
 *
 * This either adds a new entry, or updates an existing one in the set.
 * @return A new channel_hint, if the addition resulted in changes.
 */
struct channel_hint *channel_hint_set_add(struct channel_hint_set *self,
					  u32 timestamp,
					  const struct short_channel_id_dir *scidd,
					  bool enabled,
					  const struct amount_msat *estimated_capacity,
					  const struct amount_msat overall_capacity,
					  u16 *htlc_budget);

/**
 * Count how many channel_hints the set contains.
 */
size_t channel_hint_set_count(const struct channel_hint_set *set);

#endif /* LIGHTNING_PLUGINS_CHANNEL_HINT_H */
