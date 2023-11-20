; ModuleID = 'mini-c'
source_filename = "mini-c"

@z = common global i32 0, align 4

declare i32 @print_int(i32)

declare float @print_float(float)

define i32 @assign() {
entry:
  %y = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 2, ptr %y, align 4
  store i32 2, ptr %x, align 4
  store i32 3, ptr %y, align 4
  store i32 3, ptr @z, align 4
  %load_temp = load i32, ptr %x, align 4
  %load_temp1 = load i32, ptr %y, align 4
  %add_tmp = add i32 %load_temp, %load_temp1
  %load_global_temp = load i32, ptr @z, align 4
  %add_tmp2 = add i32 %add_tmp, %load_global_temp
  ret i32 %add_tmp2
}
