#include "util.h"
#include "my_log.h"

bool hex2bin(void* output, const char* hexstr, size_t len)
{
	uchar* p = (uchar*)output;
	char hex_byte[4];
	char* ep;

	hex_byte[2] = '\0';

	while (*hexstr && len) {
		if (!hexstr[1]) {
			printf("hex2bin str truncated");
			return false;
		}
		hex_byte[0] = hexstr[0];
		hex_byte[1] = hexstr[1];
		*p = (uchar)strtol(hex_byte, &ep, 16);
		if (*ep) {
			printf("hex2bin failed on '%s'", hex_byte);
			return false;
		}
		p++;
		hexstr += 2;
		len--;
	}

	return (len == 0 && *hexstr == 0) ? true : false;
}


char* bin2hex(const uchar* in, size_t len) {
	char* s = (char*)malloc((len * 2) + 1);
	if (!s)
		return NULL;

	cbin2hex(s, (const char*)in, len);

	return s;
}

void cbin2hex(char* out, const char* in, size_t len)
{
	if (out) {
		unsigned int i;
		for (i = 0; i < len; i++)
//			sprintf_s(out + (i * 2), 3, "%02x", (uint8_t)in[i]);
			snprintf(out + (i * 2), 3, "%02x", (uint8_t)in[i]);

	}
}

//double target_to_diff_verus(uint32_t target) {
//	const unsigned exponent_diff = 8 * (0x20 - ((target >> 24) & 0xFF));
//	const double significand = target & 0xFFFFFF;
//	return std::ldexp(0x0f0f0f / significand, exponent_diff);
//}

//double verus_network_diff(struct work* work)
//{
//	uint32_t nbits = work->data[26];
//
//	double d = target_to_diff_verus(nbits);
//	// applog(LOG_BLUE, "target nbits: %08x", nbits);
//	// applog(LOG_BLUE, "target diff: %f", d);
//	return d;
//}

//void equi_work_set_target(struct work* work, double diff)
//{
//	// target is given as data by the equihash stratum
//	// memcpy(work->target, stratum.job.claim, 32); // claim field is only used for lbry
//	// diff_to_target_equi(work->target, diff); // we already set the target
//	work->targetdiff = diff;
//	// applog(LOG_BLUE, "diff %f to target :", diff);
//	// applog_hex(work->target, 32);
//}

/**
 * Unlike malloc, calloc set the memory to zero
 */
void* aligned_calloc(int size)
{
	const int ALIGN = 64; // cache line
#ifdef _MSC_VER
	void* res = _aligned_malloc(size, ALIGN);
	if (res) {
		memset(res, 0, size);
	}
	return res;
#else
	void* mem = calloc(1, size + ALIGN + sizeof(uintptr_t));
	void** ptr = (void**)((size_t)(((uintptr_t)(mem)) + ALIGN + sizeof(uintptr_t)) & ~(ALIGN - 1));
	ptr[-1] = mem;
	return ptr;
#endif
}

bool send_line(curl_socket_t sock, char* s)
{
	//ssize_t len, sent = 0;

	//len = (ssize_t)strlen(s);
	//s[len++] = '\n';

	//while (len > 0) {
	//	struct timeval timeout = { 0, 0 };
	//	ssize_t n;
	//	fd_set wd;

	//	FD_ZERO(&wd);
	//	FD_SET(sock, &wd);
	//	if (select((int)sock + 1, NULL, &wd, NULL, &timeout) < 1)
	//		return false;
	//	n = send(sock, s + sent, len, 0);
	//	/*if (n < 0) {
	//		if (!socket_blocks())
	//			return false;
	//		n = 0;
	//	}*/
	//	sent += n;
	//	len -= n;
	//}

	return true;
}

pthread_mutex_t stratum_sock_lock;
pthread_mutex_t stratum_work_lock;

bool stratum_send_line(struct stratum_ctx* sctx, char* s)
{
	bool ret = false;

	/*if (opt_protocol)
		applog(LOG_DEBUG, "> %s", s);

	pthread_mutex_lock(&stratum_sock_lock);
	ret = send_line(sctx->sock, s);
	pthread_mutex_unlock(&stratum_sock_lock);*/

	return ret;
}

/**
 * Extract bloc height     L H... here len=3, height=0x1333e8
 * "...0000000000ffffffff2703e83313062f503253482f043d61105408"
 */
uint32_t getblocheight(struct stratum_ctx* sctx)
{
	uint32_t height = 0;
	uint8_t hlen = 0, * p, * m;

	// find 0xffff tag
	p = (uint8_t*)sctx->job.coinbase + 32;
	m = p + 128;
	while (*p != 0xff && p < m) p++;
	while (*p == 0xff && p < m) p++;
	if (*(p - 1) == 0xff && *(p - 2) == 0xff) {
		p++; hlen = *p;
		p++; height = le16dec(p);
		p += 2;
		switch (hlen) {
		case 4:
			height += 0x10000UL * le16dec(p);
			break;
		case 3:
			height += 0x10000UL * (*p);
			break;
		}
	}
	return height;
}

bool stratum_notify(struct stratum_ctx* sctx, json_t* params)
{
	const char* job_id, * prevhash, * coinb1, * coinb2, * version, * nbits, * stime;
	const char* extradata = NULL, * solution = NULL;;
	size_t coinb1_size, coinb2_size;
	bool clean, ret = false;
	int merkle_count, i, p = 0;
	json_t* merkle_arr;
	uchar** merkle = NULL;
	// uchar(*merkle_tree)[32] = { 0 };
	int ntime;
	char algo[64] = { 0 };
	//get_currentalgo(algo, sizeof(algo));
	strcpy(algo, "verus");
	bool has_claim = !strcmp(algo, "lbry");
	bool has_roots = !strcmp(algo, "phi2") && json_array_size(params) == 10;

	char* json_str = json_dumps(params, JSON_INDENT(4));
	//printf("params: %s \n", json_str);

	if (sctx->is_equihash) {
		return equi_stratum_notify(sctx, params);
	}

	job_id = json_string_value(json_array_get(params, p++));
	prevhash = json_string_value(json_array_get(params, p++));
	if (has_claim) {
		extradata = json_string_value(json_array_get(params, p++));
		if (!extradata || strlen(extradata) != 64) {
			//applog(LOG_ERR, "Stratum notify: invalid claim parameter");
			goto out;
		}
	}
	else if (has_roots) {
		extradata = json_string_value(json_array_get(params, p++));
		if (!extradata || strlen(extradata) != 128) {
			//applog(LOG_ERR, "Stratum notify: invalid UTXO root parameter");
			goto out;
		}
	}
	coinb1 = json_string_value(json_array_get(params, p++));
	coinb2 = json_string_value(json_array_get(params, p++));
	merkle_arr = json_array_get(params, p++);
	if (!merkle_arr || !json_is_array(merkle_arr))
		goto out;
	merkle_count = (int)json_array_size(merkle_arr);
	version = json_string_value(json_array_get(params, p++));
	nbits = json_string_value(json_array_get(params, p++));
	stime = json_string_value(json_array_get(params, p++));
	clean = json_is_true(json_array_get(params, p)); p++;
	solution = json_string_value(json_array_get(params, p++));

	if (!job_id || !prevhash || !coinb1 || !coinb2 || !version || !nbits || !stime ||
		strlen(prevhash) != 64 || strlen(version) != 8 ||
		strlen(nbits) != 8 || strlen(stime) != 8) {
		//applog(LOG_ERR, "Stratum notify: invalid parameters");
		goto out;
	}

	/* store stratum server time diff */
	hex2bin((uchar*)&ntime, stime, 4);
	ntime = swab32(ntime) - (uint32_t)time(0);
	if (ntime > sctx->srvtime_diff) {
		sctx->srvtime_diff = ntime;
		//if (opt_protocol && ntime > 20)
			//applog(LOG_DEBUG, "stratum time is at least %ds in the future", ntime);
	}

	if (merkle_count)
		merkle = (uchar**)malloc(merkle_count * sizeof(char*));
	for (i = 0; i < merkle_count; i++) {
		const char* s = json_string_value(json_array_get(merkle_arr, i));
		if (!s || strlen(s) != 64) {
			while (i--)
				free(merkle[i]);
			free(merkle);
			//applog(LOG_ERR, "Stratum notify: invalid Merkle branch");
			goto out;
		}
		merkle[i] = (uchar*)malloc(32);
		hex2bin(merkle[i], s, 32);
	}

	pthread_mutex_lock(&stratum_work_lock);

	coinb1_size = strlen(coinb1) / 2;
	coinb2_size = strlen(coinb2) / 2;
	sctx->job.coinbase_size = coinb1_size + sctx->xnonce1_size +
		sctx->xnonce2_size + coinb2_size;

	sctx->job.coinbase = (uchar*)realloc(sctx->job.coinbase, sctx->job.coinbase_size);
	sctx->job.xnonce2 = sctx->job.coinbase + coinb1_size + sctx->xnonce1_size;
	hex2bin(sctx->job.coinbase, coinb1, coinb1_size);
	memcpy(sctx->job.coinbase + coinb1_size, sctx->xnonce1, sctx->xnonce1_size);

	if (!sctx->job.job_id || strcmp(sctx->job.job_id, job_id))
		memset(sctx->job.xnonce2, 0, sctx->xnonce2_size);
	hex2bin(sctx->job.xnonce2 + sctx->xnonce2_size, coinb2, coinb2_size);

	free(sctx->job.job_id);
	sctx->job.job_id = strdup(job_id);
	hex2bin(sctx->job.prevhash, prevhash, 32);
	if (has_claim) hex2bin(sctx->job.extra, extradata, 32);
	if (has_roots) hex2bin(sctx->job.extra, extradata, 64);

	sctx->job.height = getblocheight(sctx);

	for (i = 0; i < sctx->job.merkle_count; i++)
		free(sctx->job.merkle[i]);
	free(sctx->job.merkle);
	sctx->job.merkle = merkle;
	sctx->job.merkle_count = merkle_count;

	hex2bin(sctx->job.version, version, 4);
	hex2bin(sctx->job.nbits, nbits, 4);
	hex2bin(sctx->job.ntime, stime, 4);

	hex2bin(sctx->job.solution, solution, 1344);

	sctx->job.clean = clean;

	sctx->job.diff = sctx->next_diff;

	pthread_mutex_unlock(&stratum_work_lock);

	ret = true;

out:
	return ret;
}


#define JSON_SUBMIT_BUF_LEN (20*1024)
#define EQNONCE_OFFSET 30 /* 27:34 */
// called by submit_upstream_work()
// from equi_stratum_submit
bool verus_submit(struct work* work, char** submitStr)
{
	char _ALIGN(64) s[JSON_SUBMIT_BUF_LEN];
	char _ALIGN(64) timehex[16] = { 0 };
	char* jobid, * noncestr, * solhex;
	int idnonce = work->submit_nonce_id;

	// scanned nonce
	work->data[EQNONCE_OFFSET] = work->nonces[idnonce];
	unsigned char* nonce = (unsigned char*)(&work->data[27]);
	size_t nonce_len = 32 - stratum.xnonce1_size;
	// long nonce without pool prefix (extranonce)
	noncestr = bin2hex(&nonce[stratum.xnonce1_size], nonce_len);

	solhex = (char*)calloc(1, 1344 * 2 + 64);
	if (!solhex || !noncestr) {
		//applog(LOG_ERR, "unable to alloc share memory");
		return false;
	}
	cbin2hex(solhex, (const char*)work->extra, 1347);

	char* solHexRestore = (char*)calloc(129, 1);
	cbin2hex(solHexRestore, (const char*)&work->solution[8], 64);
	memcpy(&solhex[6 + 16], solHexRestore, 128);


	jobid = work->job_id + 8;
//	sprintf_s(timehex, "%08x", swab32(work->data[25]));
    snprintf(timehex, sizeof(timehex), "%08x", swab32(work->data[25]));

	//printf("%s\n", solhex);

//	for (int i = 0; i < strlen(solhex); i += 128) {
//		char chunk[129] = {0};
//		strncpy(chunk, solhex + i, 128);
//		LOGI("sol[%d]: %s", i / 128, chunk);
//	}

//RYSXNdVZkdFRvU3ySwdA3orQ1z1tEVpvRY
//    snprintf(s, sizeof(s), "{\"method\":\"mining.submit\",\"params\":"
//                           "[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"], \"id\":%u}\n",
//             "RXxxZzRxYT68fXFPmMiR7xaQySv1hWtUNF.CPU", jobid, timehex, noncestr, solhex,
//             stratum.job.shares_count + 10);

	snprintf(s, sizeof(s), "{\"method\":\"mining.submit\",\"params\":"
						   "[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"], \"id\":%u}\n",
			 "RYSXNdVZkdFRvU3ySwdA3orQ1z1tEVpvRY.CPU", jobid, timehex, noncestr, solhex,
			 stratum.job.shares_count + 10);

//	_snprintf_s(s, sizeof(s), _TRUNCATE, "{\"method\":\"mining.submit\",\"params\":"
//		"[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"], \"id\":%u}\n",
//		"RXxxZzRxYT68fXFPmMiR7xaQySv1hWtUNF.CPU", jobid, timehex, noncestr, solhex,
//		stratum.job.shares_count + 10);
	/*printf("{\"/*method\":\"mining.s/*ubmit\",\"params\":"
		"[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"], \"id\":%u}",
		1, jobid, timehex, noncestr, solhex,
		stratum.job.shares_count + 10);*/

//    for (int i = 0; i < strlen(s); i += 128) {
//        char chunk[129] = {0};
//        strncpy(chunk, s + i, 128);
//        LOGI("sol[%d]: %s", i / 128, chunk);
//    }

	*submitStr = (char*)malloc(strlen(s) + 1);
	if (submitStr) {
//		strcpy_s(*submitStr, strlen(s) + 1, s);
        strcpy(*submitStr, s);

    }

	free(solHexRestore);
	free(solhex);
	free(noncestr);

	gettimeofday(&stratum.tv_submit, NULL);

	//if (!stratum_send_line(&stratum, s)) {
	//	//applog(LOG_ERR, "%s stratum_send_line failed", __func__);
	//	return false;
	//}

	stratum.sharediff = work->sharediff[idnonce];
	stratum.job.shares_count++;

	return true;
}

bool stratum_parse_extranonce(struct stratum_ctx* sctx, json_t* params, int pndx)
{
	const char* xnonce1;
	int xn2_size;

	xnonce1 = json_string_value(json_array_get(params, pndx));
	if (!xnonce1) {
		//applog(LOG_ERR, "Failed to get extranonce1");
		return false;
	}
	xn2_size = (int)json_integer_value(json_array_get(params, pndx + 1));
	if (!xn2_size) {
		char algo[64] = { 0 };
		int xn1_size = (int)strlen(xnonce1) / 2;
		xn2_size = 32 - xn1_size;
		if (xn1_size < 3 || xn1_size > 12) {
			// This miner iterates the nonces at data32[30]
			//applog(LOG_ERR, "Unsupported extranonce size of %d (12 maxi)", xn1_size);
			return false;
		}
		goto skip_n2;
	}
	if (xn2_size < 2 || xn2_size > 16) {
		//applog(LOG_ERR, "Failed to get valid n2size in parse_extranonce (%d)", xn2_size);
		return false;
	}
skip_n2:
	//pthread_mutex_lock(&stratum_work_lock);
	if (sctx->xnonce1)
		free(sctx->xnonce1);
	sctx->xnonce1_size = strlen(xnonce1) / 2;
	sctx->xnonce1 = (uchar*)calloc(1, sctx->xnonce1_size);
	if (unlikely(!sctx->xnonce1)) {
		//applog(LOG_ERR, "Failed to alloc xnonce1");
		//pthread_mutex_unlock(&stratum_work_lock);
		return false;
	}
	hex2bin(sctx->xnonce1, xnonce1, sctx->xnonce1_size);
	sctx->xnonce2_size = xn2_size;
	//pthread_mutex_unlock(&stratum_work_lock);

	return true;
}