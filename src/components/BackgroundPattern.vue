<template>
  <div
    class="background-pattern"
    :class="{ 'dark-mode': isDarkMode }"
  >
    <!-- 网格背景 -->
    <div class="grid-pattern" />
    
    <!-- 流体背景元素 -->
    <div class="fluid-element fluid-1" />
    <div class="fluid-element fluid-2" />
    <div class="fluid-element fluid-3" />
    
    <!-- 背景光晕 -->
    <div class="glow-effect glow-1" />
    <div class="glow-effect glow-2" />
  </div>
</template>

<script setup>
import { computed } from 'vue'
import { useStore } from 'vuex'

const store = useStore()
const isDarkMode = computed(() => store.state.theme === 'dark')
</script>

<style scoped>
.background-pattern {
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  z-index: -1;
  overflow: hidden;
}

/* 网格背景 */
.grid-pattern {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background-image: 
    linear-gradient(rgba(102, 126, 234, 0.05) 1px, transparent 1px),
    linear-gradient(90deg, rgba(102, 126, 234, 0.05) 1px, transparent 1px);
  background-size: 40px 40px;
  opacity: 0.8;
}

/* 流体背景元素 */
.fluid-element {
  position: absolute;
  border-radius: 50%;
  filter: blur(40px);
  opacity: 0.3;
  animation: float 15s infinite ease-in-out;
}

.fluid-1 {
  width: 300px;
  height: 300px;
  background: linear-gradient(135deg, #667eea, #764ba2);
  top: 20%;
  left: 10%;
  animation-delay: 0s;
}

.fluid-2 {
  width: 400px;
  height: 400px;
  background: linear-gradient(135deg, #667eea, #764ba2);
  bottom: 15%;
  right: 15%;
  animation-delay: -5s;
}

.fluid-3 {
  width: 250px;
  height: 250px;
  background: linear-gradient(135deg, #667eea, #764ba2);
  top: 60%;
  left: 70%;
  animation-delay: -10s;
}

/* 背景光晕 */
.glow-effect {
  position: absolute;
  border-radius: 50%;
  filter: blur(80px);
  opacity: 0.1;
  animation: pulse 6s infinite ease-in-out;
}

.glow-1 {
  width: 500px;
  height: 500px;
  background: linear-gradient(135deg, #667eea, transparent);
  top: 10%;
  left: 10%;
  animation-delay: 0s;
}

.glow-2 {
  width: 600px;
  height: 600px;
  background: linear-gradient(135deg, #764ba2, transparent);
  bottom: 10%;
  right: 10%;
  animation-delay: -3s;
}

/* 动画定义 */
@keyframes float {
  0%, 100% {
    transform: translateY(0) translateX(0);
  }
  25% {
    transform: translateY(-20px) translateX(20px);
  }
  50% {
    transform: translateY(0) translateX(40px);
  }
  75% {
    transform: translateY(20px) translateX(20px);
  }
}

@keyframes pulse {
  0%, 100% {
    transform: scale(1);
    opacity: 0.1;
  }
  50% {
    transform: scale(1.1);
    opacity: 0.15;
  }
}

/* 暗色模式调整 */
.background-pattern.dark-mode .grid-pattern {
  background-image: 
    linear-gradient(rgba(255, 255, 255, 0.03) 1px, transparent 1px),
    linear-gradient(90deg, rgba(255, 255, 255, 0.03) 1px, transparent 1px);
}

.background-pattern.dark-mode .fluid-element {
  background: linear-gradient(135deg, rgba(102, 126, 234, 0.3), rgba(118, 75, 162, 0.3));
}

.background-pattern.dark-mode .glow-effect {
  opacity: 0.08;
}
</style>
