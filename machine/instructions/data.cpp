#include "instructions/data.hpp"

namespace rubinius {
  const instructions::InstructionData Instructions::data[] = {
    instructions::data_add_scope,
    instructions::data_allow_private,
    instructions::data_cast_array,
    instructions::data_cast_for_multi_block_arg,
    instructions::data_cast_for_single_block_arg,
    instructions::data_cast_for_splat_block_arg,
    instructions::data_cast_multi_value,
    instructions::data_check_frozen,
    instructions::data_check_interrupts,
    instructions::data_check_serial,
    instructions::data_check_serial_private,
    instructions::data_clear_exception,
    instructions::data_create_block,
    instructions::data_dup,
    instructions::data_dup_many,
    instructions::data_ensure_return,
    instructions::data_find_const,
    instructions::data_goto,
    instructions::data_goto_if_equal,
    instructions::data_goto_if_false,
    instructions::data_goto_if_nil,
    instructions::data_goto_if_not_equal,
    instructions::data_goto_if_not_nil,
    instructions::data_goto_if_not_undefined,
    instructions::data_goto_if_true,
    instructions::data_goto_if_undefined,
    instructions::data_instance_of,
    instructions::data_invoke_primitive,
    instructions::data_kind_of,
    instructions::data_make_array,
    instructions::data_move_down,
    instructions::data_noop,
    instructions::data_object_to_s,
    instructions::data_passed_arg,
    instructions::data_passed_blockarg,
    instructions::data_pop,
    instructions::data_pop_many,
    instructions::data_pop_unwind,
    instructions::data_push_block,
    instructions::data_push_block_arg,
    instructions::data_push_const,
    instructions::data_push_cpath_top,
    instructions::data_push_current_exception,
    instructions::data_push_exception_state,
    instructions::data_push_false,
    instructions::data_push_has_block,
    instructions::data_push_int,
    instructions::data_push_ivar,
    instructions::data_push_literal,
    instructions::data_push_local,
    instructions::data_push_local_depth,
    instructions::data_push_memo,
    instructions::data_push_mirror,
    instructions::data_push_my_field,
    instructions::data_push_my_offset,
    instructions::data_push_nil,
    instructions::data_push_proc,
    instructions::data_push_rubinius,
    instructions::data_push_scope,
    instructions::data_push_self,
    instructions::data_push_stack_local,
    instructions::data_push_true,
    instructions::data_push_type,
    instructions::data_push_undef,
    instructions::data_push_variables,
    instructions::data_raise_break,
    instructions::data_raise_exc,
    instructions::data_raise_return,
    instructions::data_reraise,
    instructions::data_restore_exception_state,
    instructions::data_ret,
    instructions::data_rotate,
    instructions::data_send_method,
    instructions::data_send_stack,
    instructions::data_send_stack_with_block,
    instructions::data_send_stack_with_splat,
    instructions::data_send_super_stack_with_block,
    instructions::data_send_super_stack_with_splat,
    instructions::data_send_vcall,
    instructions::data_set_call_flags,
    instructions::data_set_const,
    instructions::data_set_const_at,
    instructions::data_set_ivar,
    instructions::data_set_local,
    instructions::data_set_local_depth,
    instructions::data_set_stack_local,
    instructions::data_setup_unwind,
    instructions::data_shift_array,
    instructions::data_store_my_field,
    instructions::data_string_append,
    instructions::data_string_build,
    instructions::data_string_dup,
    instructions::data_swap,
    instructions::data_unwind,
    instructions::data_yield_debugger,
    instructions::data_yield_splat,
    instructions::data_yield_stack,
    instructions::data_zsuper,
    instructions::data_push_file,
    instructions::data_p_any,
    instructions::data_p_call,
    instructions::data_p_char,
    instructions::data_p_char_set,
    instructions::data_p_choice,
    instructions::data_p_commit,
    instructions::data_p_commit_back,
    instructions::data_p_commit_partial,
    instructions::data_p_end,
    instructions::data_p_fail,
    instructions::data_p_fail_twice,
    instructions::data_p_jump,
    instructions::data_p_return,
    instructions::data_p_span,
    instructions::data_p_test_any,
    instructions::data_p_test_char,
    instructions::data_p_test_char_set,
    instructions::data_p_init,
    instructions::data_m_bytes,
    instructions::data_m_counter,
    instructions::data_m_sum,
    instructions::data_m_value,
    instructions::data_m_time_stamp,
    instructions::data_m_timer_start,
    instructions::data_m_timer_stop,
    instructions::data_b_if_serial,
    instructions::data_b_if_int,
    instructions::data_b_if,
    instructions::data_r_load_local,
    instructions::data_r_store_local,
    instructions::data_r_load_local_depth,
    instructions::data_r_store_local_depth,
    instructions::data_r_load_stack,
    instructions::data_r_store_stack,
    instructions::data_r_load_literal,
    instructions::data_r_load_int,
    instructions::data_r_store_int,
    instructions::data_r_copy,
    instructions::data_n_iadd,
    instructions::data_n_isub,
    instructions::data_n_imul,
    instructions::data_n_idiv,
    instructions::data_n_iadd_o,
    instructions::data_n_isub_o,
    instructions::data_n_imul_o,
    instructions::data_n_idiv_o,
    instructions::data_n_ieq,
    instructions::data_n_ine,
    instructions::data_n_ilt,
    instructions::data_n_ile,
    instructions::data_n_igt,
    instructions::data_n_ige,
    instructions::data_n_ipopcnt,
    instructions::data_m_log,
    instructions::data_m_debug,
    instructions::data_e_cache_method_p,
    instructions::data_e_cache_function_p,
    instructions::data_e_cache_value_p,
    instructions::data_e_cache_method,
    instructions::data_e_cache_function,
    instructions::data_e_cache_value,
    instructions::data_e_resolve_method,
    instructions::data_e_resolve_receiver_method,
    instructions::data_e_resolve_function,
    instructions::data_e_resolve_scope_constant,
    instructions::data_e_resolve_path_constant,
    instructions::data_e_signature,
    instructions::data_e_check_signature,
    instructions::data_e_invoke_method,
    instructions::data_e_invoke_function,
    instructions::data_a_instance,
    instructions::data_a_kind,
    instructions::data_a_method,
    instructions::data_a_receiver_method,
    instructions::data_a_type,
    instructions::data_a_function,
    instructions::data_a_equal,
    instructions::data_a_not_equal,
    instructions::data_a_less,
    instructions::data_a_less_equal,
    instructions::data_a_greater,
    instructions::data_a_greater_equal,
    instructions::data_goto_past,
    instructions::data_goto_future,
    instructions::data_r_load_0,
    instructions::data_r_load_1,
    instructions::data_r_load_nil,
    instructions::data_r_load_false,
    instructions::data_r_load_true,
    instructions::data_call_send,
    instructions::data_call,
    instructions::data_call_0,
    instructions::data_push_tagged_nil,
    instructions::data_r_load_bool,
    instructions::data_r_load_m_binops,
    instructions::data_r_load_f_binops,
    instructions::data_r_ret,
    instructions::data_n_imod,
    instructions::data_n_ineg,
    instructions::data_n_inot,
    instructions::data_n_iand,
    instructions::data_n_ior,
    instructions::data_n_ixor,
    instructions::data_n_ishl,
    instructions::data_n_ishr,
    instructions::data_n_imod_o,
    instructions::data_n_ineg_o,
    instructions::data_n_ishl_o,
    instructions::data_n_ishr_o,
    instructions::data_n_promote,
    instructions::data_n_eadd,
    instructions::data_n_esub,
    instructions::data_n_emul,
    instructions::data_n_ediv,
    instructions::data_n_emod,
    instructions::data_n_eneg,
    instructions::data_n_enot,
    instructions::data_n_eand,
    instructions::data_n_eor,
    instructions::data_n_exor,
    instructions::data_n_eshl,
    instructions::data_n_eshr,
    instructions::data_n_epopcnt,
    instructions::data_n_eeq,
    instructions::data_n_ene,
    instructions::data_n_elt,
    instructions::data_n_ele,
    instructions::data_n_egt,
    instructions::data_n_ege,
    instructions::data_n_dadd,
    instructions::data_n_dsub,
    instructions::data_n_dmul,
    instructions::data_n_ddiv,
    instructions::data_n_dmod,
    instructions::data_n_dneg,
    instructions::data_n_deq,
    instructions::data_n_dne,
    instructions::data_n_dlt,
    instructions::data_n_dle,
    instructions::data_n_dgt,
    instructions::data_n_dge,
    instructions::data_r_load_float,
    instructions::data_r_store_float,
    instructions::data_b_if_eint,
    instructions::data_b_if_float,
    instructions::data_r_load_2,
    instructions::data_r_load_neg1,
    instructions::data_n_iinc,
    instructions::data_n_idec,
    instructions::data_n_isize,
    instructions::data_n_esize,
    instructions::data_n_ibits,
    instructions::data_n_ebits,
    instructions::data_r_load_self,
    instructions::data_n_istr,
    instructions::data_n_iflt,
    instructions::data_n_icmp,
    instructions::data_n_idivmod,
    instructions::data_n_ipow_o,
    instructions::data_n_estr,
    instructions::data_n_eflt,
    instructions::data_n_ecmp,
    instructions::data_n_edivmod,
    instructions::data_n_epow,
    instructions::data_n_dstr,
    instructions::data_n_dcmp,
    instructions::data_n_ddivmod,
    instructions::data_n_dpow,
    instructions::data_n_demote,
    instructions::data_n_dinf,
    instructions::data_n_dnan,
    instructions::data_n_dclass,
  };
}
