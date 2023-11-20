; ModuleID = 'mini-c'
source_filename = "mini-c"

@a = common global i32 0, align 4

declare i32 @print_int(i32)

define i32 @foo() {
entry:
  %load_global_temp = load i32, ptr @a, align 4
  %add_tmp = add i32 %load_global_temp, 1
  store i32 %add_tmp, ptr @a, align 4
  %load_global_temp1 = load i32, ptr @a, align 4
  ret i32 %load_global_temp1
}

define i32 @global() {
entry:
  store i32 5, ptr @a, align 4
  %call_tmp = call i32 @foo()
  ret i32 %call_tmp
}
