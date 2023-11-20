; ModuleID = 'mini-c'
source_filename = "mini-c"

define i32 @associativity() {
entry:
  %result = alloca i32, align 4
  %m = alloca i32, align 4
  %n = alloca i32, align 4
  store i32 4, ptr %n, align 4
  store i32 5, ptr %m, align 4
  %load_temp = load i32, ptr %n, align 4
  %load_temp1 = load i32, ptr %m, align 4
  %sub_tmp = sub i32 %load_temp, %load_temp1
  %sub_tmp2 = sub i32 %sub_tmp, 3
  store i32 %sub_tmp2, ptr %result, align 4
  %load_temp3 = load i32, ptr %result, align 4
  ret i32 %load_temp3
}
