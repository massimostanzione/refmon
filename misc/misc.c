#include <linux/kernel.h>
#include <linux/module.h>
#include <crypto/hash.h>
#include <crypto/sha.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/err.h>
#include "../refmon.h"
#include "misc.h"

#define HASH_STRING_SIZE (2 * SHA256_DIGEST_SIZE + 1)

static void hexconv(const u8 *hash, char *out, int hash_size)
{
	int i;
	for (i = 0; i < hash_size; i++) {
		sprintf(out + (i * 2), "%02x", hash[i]);
	}
	out[hash_size * 2] = '\0';
}

static struct shash_desc *alloc_hash_desc(struct crypto_shash **shash)
{
	struct shash_desc *desc;

	*shash = crypto_alloc_shash(REFMON_CRYPT_ALGO, 0, 0);
	if (IS_ERR(*shash)) {
		pr_err("%s: error while trying to allocate shash ctx",
		       REFMON_MODNAME);
		goto tail_err;
	}

	desc = kmalloc(sizeof(*desc) + crypto_shash_descsize(*shash),
		       GFP_KERNEL);
	if (desc == NULL) {
		pr_err("%s: error while trying to allocate shash desc",
		       REFMON_MODNAME);
		crypto_free_shash(*shash);
		goto tail_err;
	}

	desc->tfm = *shash;
	return desc;

tail_err:
	return NULL;
}

static int shash_comp(struct shash_desc *desc, const char *input, size_t len,
		      u8 *hash)
{
	int ret = REFMON_RETVAL_DEFAULT;

	ret = crypto_shash_init(desc);
	if (ret) {
		pr_err("%s: error while trying to init shash", REFMON_MODNAME);
		goto tail;
	}

	ret = crypto_shash_update(desc, input, len);
	if (ret) {
		pr_err("%s: error while trying to update shash",
		       REFMON_MODNAME);
		goto tail;
	}

	ret = crypto_shash_final(desc, hash);
	if (ret) {
		pr_err("%s: error while trying to do the final shash comp",
		       REFMON_MODNAME);
	}
tail:
	return ret;
}

char *hashgen_str(const char *str)
{
	struct crypto_shash *shash;
	struct shash_desc *desc;
	char *out;
	u8 hash[SHA256_DIGEST_SIZE];
	int ret = REFMON_RETVAL_DEFAULT;

	out = kmalloc(HASH_STRING_SIZE, GFP_KERNEL);
	if (!out) {
		pr_err("%s: error while trying to kmalloc", REFMON_MODNAME);
		goto tail_err;
	}

	desc = alloc_hash_desc(&shash);
	if (desc == NULL) {
		pr_err("%s: error while trying to allocate shash ctx",
		       REFMON_MODNAME);
		goto tail_err;
	}

	ret = shash_comp(desc, str, strlen(str), hash);
	if (ret) {
		crypto_free_shash(shash);
		goto tail_err;
	}

	hexconv(hash, out, SHA256_DIGEST_SIZE);

	crypto_free_shash(shash);
	goto tail;
tail_err:
	out = NULL;
tail:
	FREE(desc);
	return out;
}

char *hashgen_filecont(const char *filepath)
{
	struct file *file;
	mm_segment_t old_fs;
	struct crypto_shash *shash;
	struct shash_desc *desc;
	u8 hash[SHA256_DIGEST_SIZE];
	char *out;
	char *buft;
	loff_t offset = 0;
	int ret;
	const int buf_size = 4096;

	out = kmalloc(HASH_STRING_SIZE, GFP_KERNEL);
	if (out == NULL) {
		pr_err("%s: error while trying to kmalloc (filepath='%s')",
		       REFMON_MODNAME, filepath);
		goto tail_err;
	}

	buft = kmalloc(buf_size, GFP_KERNEL);
	if (buft == NULL) {
		pr_err("%s: error while trying to kmalloc", REFMON_MODNAME);
		FREE(out);
		goto tail_err;
	}

	desc = alloc_hash_desc(&shash);
	if (desc == NULL)
		goto tail_err;

	ret = crypto_shash_init(desc);
	if (ret) {
		pr_err("%s: error while trying to init shash ctx",
		       REFMON_MODNAME);
		goto tail;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	file = filp_open(filepath, O_RDONLY, 0);
	set_fs(old_fs);
	if (IS_ERR(file)) {
		pr_err("%s: error while trying to open file (filepath='%s')",
		       REFMON_MODNAME, filepath);
		ret = PTR_ERR(file);
		goto tail;
	}

	while ((ret = kernel_read(file, buft, buf_size, &offset)) > 0) {
		ret = crypto_shash_update(desc, buft, ret);
		if (ret) {
			pr_err("%s: error while trying to update shash during file read",
			       REFMON_MODNAME);
			goto tail_err;
		}
	}

	if (ret < 0) {
		pr_err("%s: error while trying to read file (shash)",
		       REFMON_MODNAME);
		goto tail_err;
	}

	ret = crypto_shash_final(desc, hash);
	if (ret) {
		pr_err("%s: error while trying to compute final shash (filename='%s')",
		       REFMON_MODNAME, filepath);
		goto tail_err;
	}

	hexconv(hash, out, SHA256_DIGEST_SIZE);

tail_err:
	filp_close(file, NULL);

tail:
	FREE(desc);
	crypto_free_shash(shash);
	FREE(buft);

	if (ret) {
		FREE(out);
		return NULL;
	}

	return out;
}

int check_list_init(struct list_head *list)
{
	return list->next == list && list->prev == list;
}
