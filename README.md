# MetaSim
Universal Simulation Base for MPM

基本 模拟/动画 工具集

## 一些设计构想

### 粒子 Particle属性

+ 所有的粒子属性是通过 <属性名, 数组> 存储在data表内部
+ 每个粒子有专属的 group ，group决定了更新规则？
  + 基本的particles有 mass, volume, velocity, position 等属性，对于这一类基本属性进行共同更新，例如 advect，project，P2G，G2P等过程，即最基本的流体解算过程。
  + 对于不同粒子的异质属性，需要定义组别的更新过程。
    + 例如组间会有不同的 plasticity，有几种解决思路
      1. 对于每个 group 定义一个 tag 标识，那么在 ApplyPlastic 时根据 tag 进入不同的分支更新应变
      2. 维护一个 Range 数组存储不同 Plastic 的更新范围(粒子的下标)，在 ApplyPlastic 对每个 Range 内的粒子使用对应的 Plasticity 进行处理
  + ~~最好实现同质的 Group 合并， 异质的 Group 分离的数据结构~~
  + 上一条是不可能的，每个group都可以视作独立的 entity，要在同质的entity间做instancing，略困难。本质上每个粒子都是 Particle -> MPMParticle -> <不同塑性>_MPMParticle

### 网格 Grid 属性

+ 没有什么特殊的，除非接入 SPGrid
+ 切忌过度设计，暴露 iterate 方法，并需要提供插值 kernel
+ Grid 不需要与 插值 Kernel 绑定

### 最终工作流的设想

+ 希望能够通过 json/xml/其他 等配置语言实现场景以及动画的设置







