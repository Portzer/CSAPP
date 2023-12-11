0000000000001129 <add>:
    112d:	55                   	push   %rbp
    112e:	48 89 e5             	mov    %rsp,%rbp
    1131:	48 89 7d e8          	mov    %rdi,-0x18(%rbp)
    1135:	48 89 75 e0          	mov    %rsi,-0x20(%rbp)
    1139:	48 8b 55 e8          	mov    -0x18(%rbp),%rdx
    113d:	48 8b 45 e0          	mov    -0x20(%rbp),%rax
    1141:	48 01 d0             	add    %rdx,%rax
    1144:	48 89 45 f8          	mov    %rax,-0x8(%rbp)
    1148:	48 8b 45 f8          	mov    -0x8(%rbp),%rax
    114c:	5d                   	pop    %rbp
    114d:	c3                   	retq

    1173:	48 89 d6             	mov    %rdx,%rsi
    1176:	48 89 c7             	mov    %rax,%rdi
    1179:	e8 ab ff ff ff       	callq  1129 <add>
    117e:	48 89 45 f8          	mov    %rax,-0x8(%rbp)