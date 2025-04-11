#include <jansson.h>
#include "miner.h"
#include "util.h"
#include "compat.h"
#include "equihash.h"

bool opt_showdiff = true;
double opt_max_diff = -1.;
static double opt_difficulty = 1.;
double   stratum_diff = 0.0;
bool opt_protocol = false;

json_t* parse_json(const char* json_str)
{
	json_error_t err;
	json_t* val = JSON_LOADS(json_str, &err);

	if (!val) {
		printf("JSON decode failed(%d): %s", err.line, err.text);
		return NULL;
	}
	return val;
}

void print_json(json_t* json) {
	if (!json) {
		printf("JSON is NULL\n");
	}

	char* json_str = json_dumps(json, JSON_INDENT(4));
	if (json_str) {
		printf("%s\n", json_str);
		free(json_str);
	}
	else {
		printf("Failed to convert JSON to string\n");
	}
}

bool stratum_gen_work(struct stratum_ctx* sctx, struct work* work)
{
	uchar merkle_root[64] = { 0 };
	int i;

	if (!sctx->job.job_id) {
		// applog(LOG_WARNING, "stratum_gen_work: job not yet retrieved");
		return false;
	}


	// store the job ntime as high part of jobid
//	sprintf_s(work->job_id, sizeof(work->job_id), "%07x %s",
//		be32dec(sctx->job.ntime) & 0xfffffff, sctx->job.job_id);
	snprintf(work->job_id, sizeof(work->job_id), "%07x %s",
			 be32dec(sctx->job.ntime) & 0xfffffff, sctx->job.job_id);


	work->xnonce2_len = sctx->xnonce2_size;
	memcpy(work->xnonce2, sctx->job.xnonce2, sctx->xnonce2_size);

	// also store the block number
	work->height = sctx->job.height;
	// and the pool of the current stratum
	work->pooln = sctx->pooln;

	for (i = 0; i < sctx->job.merkle_count; i++) {
		memcpy(merkle_root + 32, sctx->job.merkle[i], 32);
#ifdef WITH_HEAVY_ALGO
		if (opt_algo == ALGO_HEAVY || opt_algo == ALGO_MJOLLNIR)
			heavycoin_hash(merkle_root, merkle_root, 64);
		else
#endif

	}

	/* Increment extranonce2 */
	for (i = 0; i < (int)sctx->xnonce2_size && !++sctx->job.xnonce2[i]; i++);

	/* Assemble block header */
	memset(work->data, 0, sizeof(work->data));
	work->data[0] = le32dec(sctx->job.version);
	for (i = 0; i < 8; i++)
		work->data[1 + i] = le32dec((uint32_t*)sctx->job.prevhash + i);

	memcpy(&work->data[9], sctx->job.coinbase, 32 + 32); // merkle [9..16] + reserved
	work->data[25] = le32dec(sctx->job.ntime);
	work->data[26] = le32dec(sctx->job.nbits);
	memcpy(&work->solution, sctx->job.solution, 1344);
	memcpy(&work->data[27], sctx->xnonce1, sctx->xnonce1_size & 0x1F); // pool extranonce
	work->data[35] = 0x80;
	//applog_hex(work->data, 140);

	if (opt_showdiff || opt_max_diff > 0.)
		verus_network_diff(work);

	if (opt_difficulty == 0.)
		opt_difficulty = 1.;

	memcpy(work->target, sctx->job.extra, 32);
	equi_work_set_target(work, sctx->job.diff / opt_difficulty);

	if (stratum_diff != sctx->job.diff) {
		char sdiff[32] = { 0 };
		// store for api stats
		stratum_diff = sctx->job.diff;
		if (opt_showdiff && work->targetdiff != stratum_diff)
			printf(sdiff, 32, " (%.5f)", work->targetdiff);
		printf("Stratum difficulty set to %g%s", stratum_diff, sdiff);
	}

	return true;
}

bool submit_upstream_work(struct work* work, char** submitStr)
{
	char s[512];
	json_t* val, * res, * reason;
	bool stale_work = false;
	int idnonce = work->submit_nonce_id;
	
	struct work submit_work;
	memcpy(&submit_work, work, sizeof(struct work));
	
	verus_submit(&submit_work, submitStr);
	return true;
	
}