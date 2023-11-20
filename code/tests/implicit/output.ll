; ModuleID = 'mini-c'
source_filename = "mini-c"

define i32 @foo(i32 %X) {
entry:
  %X1 = alloca i32, align 4
  store i32 %X, ptr %X1, align 4
  %load_temp = load i32, ptr %X1, align 4
  ret i32 %load_temp
}

define i32 @implicit() {
entry:
  %call_tmp = call i32 @foo(i32 1)
  ret i32 %call_tmp
}
