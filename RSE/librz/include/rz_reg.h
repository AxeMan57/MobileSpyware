#ifndef RZ_REG_H
#define RZ_REG_H

#include <rz_types.h>
#include <rz_list.h>
#include <rz_util/rz_hex.h>
#include <rz_util/rz_bitvector.h>
#include <rz_util/rz_assert.h>
#include <rz_util/ht_sp.h>

#ifdef __cplusplus
extern "C" {
#endif

RZ_LIB_VERSION_HEADER(rz_reg);

/*
 * various CPUs have registers within various types/classes
 * this enum aims to cover them all.
 */
typedef enum {
	RZ_REG_TYPE_GPR, //< General purpose
	RZ_REG_TYPE_DRX,
	RZ_REG_TYPE_FPU, //< Floating point
	RZ_REG_TYPE_MMX,
	RZ_REG_TYPE_XMM,
	RZ_REG_TYPE_YMM,
	RZ_REG_TYPE_FLG,
	RZ_REG_TYPE_SEG,
	RZ_REG_TYPE_SYS, //< System
	RZ_REG_TYPE_SEC,
	RZ_REG_TYPE_VC, //< Vector
	RZ_REG_TYPE_VCC, //< Vector control
	RZ_REG_TYPE_CTR, //< Control
	RZ_REG_TYPE_LAST,
	RZ_REG_TYPE_ANY = -1
} RzRegisterType;

/*
 * pretty much all CPUs share some common registers
 * this enum aims to create an abstraction to ease cross-arch handling.
 */
typedef enum {
	RZ_REG_NAME_PC, // program counter
	RZ_REG_NAME_SP, // stack pointer
	RZ_REG_NAME_SR, // status register
	RZ_REG_NAME_BP, // base pointer
	RZ_REG_NAME_LR, // link register
	/* args */
	RZ_REG_NAME_A0, // arguments
	RZ_REG_NAME_A1,
	RZ_REG_NAME_A2,
	RZ_REG_NAME_A3,
	RZ_REG_NAME_A4,
	RZ_REG_NAME_A5,
	RZ_REG_NAME_A6,
	RZ_REG_NAME_A7,
	RZ_REG_NAME_A8,
	RZ_REG_NAME_A9,
	/* retval */
	RZ_REG_NAME_R0, // return registers
	RZ_REG_NAME_R1,
	RZ_REG_NAME_R2,
	RZ_REG_NAME_R3,
	/* flags */
	RZ_REG_NAME_ZF,
	RZ_REG_NAME_SF,
	RZ_REG_NAME_CF,
	RZ_REG_NAME_OF,
	/* syscall number (orig_eax,rax,r0,x0) */
	RZ_REG_NAME_SN,
	RZ_REG_NAME_LAST,
} RzRegisterId;

// TODO: use enum here?
#define RZ_REG_COND_EQ       0
#define RZ_REG_COND_NE       1
#define RZ_REG_COND_CF       2
#define RZ_REG_COND_CARRY    2
#define RZ_REG_COND_NEG      3
#define RZ_REG_COND_NEGATIVE 3
#define RZ_REG_COND_OF       4
#define RZ_REG_COND_OVERFLOW 4
// unsigned
#define RZ_REG_COND_HI  5
#define RZ_REG_COND_HE  6
#define RZ_REG_COND_LO  7
#define RZ_REG_COND_LOE 8
// signed
#define RZ_REG_COND_GE   9
#define RZ_REG_COND_GT   10
#define RZ_REG_COND_LT   11
#define RZ_REG_COND_LE   12
#define RZ_REG_COND_LAST 13

typedef struct {
	RzRegisterId role; ///< Register role (PC, Argument etc.)
	char *alias; ///< Alias of register.
	char *reg_name; ///< Real register name of alias.
} RzRegProfileAlias;

typedef struct {
	RzRegisterType type; ///< Reg type.
	RzRegisterType arena_type; ///< The arena the register belongs to (e.g. flags belong to gpr; xmm into fpu).
	char *name; ///< Register name.
	ut32 size; ///< Register size in bits.
	ut32 packed; ///< Packet size of register in tes.
	ut32 offset; ///< Offset into profile in bits.
	char *comment; ///< Comment about register.
	char *flags; ///< String describing the flags of the register.
} RzRegProfileDef;

typedef struct {
	RzList /*<RzRegProfileAlias *>*/ *alias;
	RzList /*<RzRegProfileDef *>*/ *defs;
} RzRegProfile;

typedef struct rz_reg_item_t {
	char *name;
	RzRegisterType type; ///< Register type.
	int size; ///< in bits> 8,16,32,64 ... 128/256
	int offset; ///< Offset into register profile in bits.
	int packed_size; ///< 0 means no packed register, 1byte pack, 2b pack...
	bool is_float; ///< Flag for float registers.
	char *flags;
	char *comment; ///< Comment to register.
	int index; ///< Index in register profile.
	int arena; ///< In which arena is this reg living. Usually equals type.
} RzRegItem;

typedef struct rz_reg_arena_t {
	ut8 *bytes;
	int size;
} RzRegArena;

typedef struct rz_reg_set_t {
	RzRegArena *arena;
	RzList /*<RzRegArena *>*/ *pool; ///< RzRegArena
	RzList /*<RzRegItem *>*/ *regs; ///< RzRegItem
	HtSP *ht_regs; ///< name:RzRegItem
	RzListIter /*<RzRegArena *>*/ *cur;
	ut32 maskregstype; ///< which type of regs has this register set (logic mask with 1 << RZ_REG_TYPE_XXX)
} RzRegSet;

typedef struct rz_reg_t {
	char *profile;
	char *reg_profile_cmt;
	char *reg_profile_str;
	RzRegProfile reg_profile;
	char *name[RZ_REG_NAME_LAST]; // aliases
	RzRegSet regset[RZ_REG_TYPE_LAST];
	RzList /*<RzRegItem *>*/ *allregs;
	RzList /*<char *>*/ *roregs;
	int iters;
	int arch;
	int bits;
	int size;
	bool is_thumb;
	bool big_endian;
} RzReg;

typedef struct rz_reg_flags_t {
	bool s; // sign, negative number (msb)
	bool z; // zero
	bool a; // half-carry adjust (if carry happens at nibble level)
	bool c; // carry
	bool o; // overflow
	bool p; // parity (lsb)
} RzRegFlags;

#ifdef RZ_API
RZ_API void rz_reg_free(RzReg *reg);
RZ_API void rz_reg_free_internal(RzReg *reg, bool init);
RZ_API RzReg *rz_reg_new(void);
RZ_API bool rz_reg_set_name(RZ_NONNULL RzReg *reg, RzRegisterId role, RZ_NONNULL const char *name);
RZ_API bool rz_reg_set_profile_string(RZ_NONNULL RzReg *reg, RZ_NONNULL const char *profile);
RZ_API char *rz_reg_profile_to_cc(RzReg *reg);
RZ_API bool rz_reg_set_reg_profile(RZ_BORROW RzReg *reg);
RZ_API bool rz_reg_set_profile(RzReg *reg, const char *profile);
RZ_API char *rz_reg_parse_gdb_profile(const char *profile);
RZ_API bool rz_reg_is_readonly(RzReg *reg, RzRegItem *item);

RZ_API RzRegSet *rz_reg_regset_get(RzReg *r, int type);
RZ_API ut64 rz_reg_getv(RzReg *reg, const char *name);
RZ_API ut64 rz_reg_getv_by_role_or_name(RzReg *reg, const char *name);
RZ_API ut64 rz_reg_setv(RzReg *reg, const char *name, ut64 val);
RZ_API const char *rz_reg_32_to_64(RzReg *reg, const char *rreg32);
RZ_API const char *rz_reg_64_to_32(RzReg *reg, const char *rreg64);
RZ_API const char *rz_reg_get_name_by_type(RzReg *reg, const char *name);
RZ_API const char *rz_reg_get_type(int idx);
RZ_API const char *rz_reg_get_name(RzReg *reg, int kind);
RZ_API RzRegItem *rz_reg_get_by_role(RzReg *reg, RzRegisterId role);
RZ_API const char *rz_reg_get_role(int role);
RZ_API int rz_reg_role_by_name(RZ_NONNULL const char *str);
RZ_API RzRegItem *rz_reg_get(RzReg *reg, const char *name, int type);
RZ_API RzRegItem *rz_reg_get_by_role_or_name(RzReg *reg, const char *name);
RZ_API const RzList /*<RzRegItem *>*/ *rz_reg_get_list(RzReg *reg, int type);
RZ_API RzRegItem *rz_reg_get_at(RzReg *reg, int type, int regsize, int delta);
RZ_API RzRegItem *rz_reg_next_diff(RzReg *reg, int type, const ut8 *buf, int buflen, RzRegItem *prev_ri, int regsize);

RZ_API void rz_reg_reindex(RzReg *reg);
RZ_API RzRegItem *rz_reg_index_get(RzReg *reg, int idx);

/* Item */
RZ_API void rz_reg_item_free(RzRegItem *item);

/* XXX: dupped ?? */
RZ_API int rz_reg_type_by_name(const char *str);
RZ_API int rz_reg_get_name_idx(const char *type);

RZ_API RzRegItem *rz_reg_cond_get(RzReg *reg, const char *name);
RZ_API void rz_reg_cond_apply(RzReg *r, RzRegFlags *f);
RZ_API bool rz_reg_cond_set(RzReg *reg, const char *name, bool val);
RZ_API int rz_reg_cond_get_value(RzReg *r, const char *name);
RZ_API bool rz_reg_cond_bits_set(RzReg *r, int type, RzRegFlags *f, bool v);
RZ_API int rz_reg_cond_bits(RzReg *r, int type, RzRegFlags *f);
RZ_API RzRegFlags *rz_reg_cond_retrieve(RzReg *r, RzRegFlags *f);
RZ_API int rz_reg_cond(RzReg *r, int type);

/* bitvector, for everything */
RZ_API RZ_OWN RzBitVector *rz_reg_get_bv(RZ_NONNULL RzReg *reg, RZ_NONNULL RzRegItem *item);
RZ_API bool rz_reg_set_bv(RZ_NONNULL RzReg *reg, RZ_NONNULL RzRegItem *item, RZ_NONNULL const RzBitVector *bv);

/* integer value 8-64 bits */
RZ_API ut64 rz_reg_get_value(RZ_NONNULL RzReg *reg, RZ_NONNULL RzRegItem *item);
RZ_API bool rz_reg_set_value(RZ_NONNULL RzReg *reg, RZ_NONNULL RzRegItem *item, ut64 value);
RZ_API ut64 rz_reg_get_value_by_role(RZ_NONNULL RzReg *reg, RzRegisterId role);
RZ_API bool rz_reg_set_value_by_role(RZ_NONNULL RzReg *reg, RzRegisterId role, ut64 value);

/* byte arena */
RZ_API RZ_OWN ut8 *rz_reg_get_bytes(RZ_NONNULL RzReg *reg, int type, RZ_NULLABLE int *size);
RZ_API bool rz_reg_set_bytes(RzReg *reg, int type, const ut8 *buf, const int len);
RZ_API bool rz_reg_read_regs(RzReg *reg, ut8 *buf, const int len);
RZ_API int rz_reg_arena_set_bytes(RzReg *reg, const char *str);
RZ_API RzRegArena *rz_reg_arena_new(size_t size);
RZ_API void rz_reg_arena_free(RzRegArena *ra);
RZ_API int rz_reg_fit_arena(RzReg *reg);
RZ_API void rz_reg_arena_swap(RzReg *reg, int copy);
RZ_API int rz_reg_arena_push(RzReg *reg);
RZ_API void rz_reg_arena_pop(RzReg *reg);
RZ_API void rz_reg_arena_zero(RzReg *reg, RzRegisterType type);

RZ_API ut8 *rz_reg_arena_peek(RzReg *reg);
RZ_API void rz_reg_arena_poke(RzReg *reg, const ut8 *buf);
RZ_API ut8 *rz_reg_arena_dup(RzReg *reg, const ut8 *source);
RZ_API const char *rz_reg_cond_to_string(int n);
RZ_API int rz_reg_cond_from_string(const char *str);
RZ_API void rz_reg_arena_shrink(RzReg *reg);

RZ_API RZ_OWN RzList /*<RzRegItem *>*/ *rz_reg_filter_items_covered(RZ_BORROW RZ_NONNULL const RzList /*<RzRegItem *>*/ *regs);

#ifdef __cplusplus
}
#endif

#endif
#endif
