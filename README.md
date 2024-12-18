# EGL融合框架
EGL融合框架使用linux操作系统上的EGL提供Android平台的EGL实现，从而利用linux上的显卡硬件驱动来支持OpenFDE中的图形显示系统的硬件渲染。

## 实现原理
1. 采用linux与图形无关平台来使用Android平台功能，如：使用surfaceless或gbm平台
2. 采用pbuffer surface来使用Android window surface功能
3. 采用gbm实现gralloc的GPU内存管理（不在此项目中）

## 约束
- linux EGL实现版本在1.4以上，必须支持EGL_EXT_image_dma_buf_import扩展
- linux采用gbm管理GPU内存，提供gbm接口实现

## 注意
- mesa驱动提供的gbm平台不支持pbuffer surface，由surfaceless提供pbuffer surface
- mwv207驱动提供的gbm平台支持pbuffer surface和window surface，不支持surfaceless平台