<template>
  <div class="official-website">
    <!-- Navigation Bar -->
    <nav class="navbar" :class="{ 'scrolled': isScrolled }">
      <div class="nav-container">
        <div class="nav-brand">
          <img :src="`/logo.png?v=${Date.now()}`" alt="智能低空交通系统" class="nav-logo" />
        </div>

        <!-- Desktop Menu -->
        <div class="nav-menu">
          <a href="#hero" class="nav-link">首页</a>
          <a href="#info" class="nav-link">方案</a>
          <a href="#features" class="nav-link">技术</a>
          <a href="#footer" class="nav-link">关于</a>
          <button class="login-btn" @click="goToLogin">登录</button>
        </div>

        <!-- Mobile Menu Toggle -->
        <label class="hamburger">
          <input type="checkbox" ref="mobileToggle">
          <span></span>
          <span></span>
          <span></span>
          <div class="mobile-menu">
            <a href="#hero" @click="closeMobileMenu">首页</a>
            <a href="#info" @click="closeMobileMenu">方案</a>
            <a href="#features" @click="closeMobileMenu">技术</a>
            <a href="#footer" @click="closeMobileMenu">关于</a>
            <button class="mobile-login-btn" @click="goToLogin">登录</button>
          </div>
        </label>
      </div>
    </nav>

    <!-- Hero Section -->
    <section id="hero" class="hero">
      <div class="hero-background"></div>
      <div class="hero-content">
        <h1 class="hero-title">智能低空交通管理</h1>
        <p class="hero-subtitle">为城市空中交通提供全方位解决方案，让未来出行更安全、更高效</p>
        <div class="hero-actions">
          <button class="cta-primary">了解更多</button>
          <button class="cta-secondary">观看演示</button>
        </div>
      </div>
    </section>

    <!-- Info Hierarchy -->
    <section id="info" class="info-section">
      <div class="container">
        <div class="info-grid">
          <div class="info-card">
            <div class="info-icon">
              <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
                <path d="M9 12l2 2 4-4"/>
                <circle cx="12" cy="12" r="9"/>
              </svg>
            </div>
            <h3>安全可靠</h3>
            <p>多重安全保障机制，确保每一次飞行的绝对安全</p>
          </div>

          <div class="info-card">
            <div class="info-icon">
              <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
                <circle cx="12" cy="12" r="10"/>
                <polyline points="12,6 12,12 16,14"/>
              </svg>
            </div>
            <h3>实时监控</h3>
            <p>24小时不间断监控，实时掌握空域状况</p>
          </div>

          <div class="info-card">
            <div class="info-icon">
              <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
                <polygon points="13,2 3,14 12,14 11,22 21,10 12,10"/>
              </svg>
            </div>
            <h3>高效智能</h3>
            <p>AI算法优化航路规划，提升空域利用效率</p>
          </div>

          <div class="info-card">
            <div class="info-icon">
              <svg width="48" height="48" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.5">
                <circle cx="12" cy="12" r="3"/>
                <path d="M12 1v6m0 6v6m11-7h-6m-6 0H1"/>
              </svg>
            </div>
            <h3>精准定位</h3>
            <p>厘米级定位精度，确保航行器精确导航</p>
          </div>
        </div>
      </div>
    </section>

    <!-- Feature Areas -->
    <section id="features" class="features-section">
      <div class="container">
        <!-- Feature 1 -->
        <div class="feature-row">
          <div class="feature-image">
            <img src="https://images.unsplash.com/photo-1551731409-43eb3e517a1a?w=800&q=80" alt="空中交通管理" />
          </div>
          <div class="feature-content">
            <h2>全域空中交通管理</h2>
            <p>统一管理城市低空空域，实现无人机、飞行汽车等航行器的有序运行。通过先进的空域规划算法，最大化空域利用效率，确保城市空中交通的安全有序。</p>
            <ul>
              <li>多层次空域规划</li>
              <li>动态航路调整</li>
              <li>冲突检测与避让</li>
            </ul>
          </div>
        </div>

        <!-- Feature 2 -->
        <div class="feature-row reverse">
          <div class="feature-image">
            <img src="https://images.unsplash.com/photo-1473773508845-188df298d2d1?w=800&q=80" alt="实时监控系统" />
          </div>
          <div class="feature-content">
            <h2>智能监控预警系统</h2>
            <p>基于人工智能的实时监控系统，能够识别异常飞行行为，预测潜在风险，并及时发出预警。为城市空中交通安全提供全方位保障。</p>
            <ul>
              <li>AI异常行为识别</li>
              <li>实时风险评估</li>
              <li>自动应急响应</li>
            </ul>
          </div>
        </div>

        <!-- Feature 3 -->
        <div class="feature-row">
          <div class="feature-image">
            <img src="https://images.unsplash.com/photo-1451187580459-43490279c0fa?w=800&q=80" alt="数据分析平台" />
          </div>
          <div class="feature-content">
            <h2>大数据分析平台</h2>
            <p>收集和分析海量飞行数据，为城市规划、政策制定提供科学依据。通过深度学习不断优化系统性能，推动智慧城市建设。</p>
            <ul>
              <li>多源数据融合</li>
              <li>智能决策支持</li>
              <li>预测性维护</li>
            </ul>
          </div>
        </div>
      </div>
    </section>

    <!-- Footer -->
    <footer id="footer" class="footer">
      <div class="container">
        <div class="footer-grid">
          <div class="footer-column">
            <h4>产品</h4>
            <ul>
              <li><a href="#">空域管理系统</a></li>
              <li><a href="#">监控预警平台</a></li>
              <li><a href="#">数据分析工具</a></li>
              <li><a href="#">移动端应用</a></li>
            </ul>
          </div>

          <div class="footer-column">
            <h4>解决方案</h4>
            <ul>
              <li><a href="#">城市级部署</a></li>
              <li><a href="#">区域性管理</a></li>
              <li><a href="#">企业级服务</a></li>
              <li><a href="#">定制化开发</a></li>
            </ul>
          </div>

          <div class="footer-column">
            <h4>开发者</h4>
            <ul>
              <li><a href="#">API 文档</a></li>
              <li><a href="#">SDK 下载</a></li>
              <li><a href="#">开发指南</a></li>
              <li><a href="#">技术支持</a></li>
            </ul>
          </div>

          <div class="footer-column">
            <h4>联系我们</h4>
            <ul>
              <li><a href="#">销售咨询</a></li>
              <li><a href="#">技术支持</a></li>
              <li><a href="#">合作伙伴</a></li>
              <li><a href="#">媒体中心</a></li>
            </ul>
          </div>
        </div>

        <div class="footer-bottom">
          <p>&copy; 2024 智能低空交通系统. 保留所有权利.</p>
        </div>
      </div>
    </footer>
  </div>
</template>

<script setup>
import { ref, onMounted, onUnmounted } from 'vue'
import { useRouter } from 'vue-router'

const router = useRouter()
const isScrolled = ref(false)
const mobileToggle = ref(null)

const handleScroll = () => {
  isScrolled.value = window.scrollY > 50
}

const goToLogin = () => {
  router.push('/login')
}

const closeMobileMenu = () => {
  if (mobileToggle.value) {
    mobileToggle.value.checked = false
  }
}

onMounted(() => {
  window.addEventListener('scroll', handleScroll)
})

onUnmounted(() => {
  window.removeEventListener('scroll', handleScroll)
})
</script>

<style scoped>
/* Apple-style Official Website */
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

.official-website {
  font-family: -apple-system, BlinkMacSystemFont, "SF Pro Display", "Helvetica Neue", sans-serif;
  line-height: 1.5;
  color: #1d1d1f;
  position: relative;
  z-index: 1;
}

.container {
  max-width: 1200px;
  margin: 0 auto;
  padding: 0 20px;
}

/* Navigation Bar */
.navbar {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  z-index: 1000;
  background: rgba(255, 255, 255, 0.8);
  backdrop-filter: blur(12px);
  border-bottom: 1px solid rgba(0, 0, 0, 0.05);
  transition: all 0.3s ease;
}

.navbar.scrolled {
  background: rgba(255, 255, 255, 0.95);
  box-shadow: 0 2px 16px rgba(0, 0, 0, 0.08);
}

.nav-container {
  max-width: 1200px;
  margin: 0 auto;
  padding: 0 20px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  height: 60px;
}

.nav-brand .nav-logo {
  height: 32px;
  width: auto;
}

.nav-menu {
  display: flex;
  align-items: center;
  gap: 32px;
}

.nav-link {
  text-decoration: none;
  color: #1d1d1f;
  font-size: 16px;
  font-weight: 400;
  transition: color 0.2s ease;
}

.nav-link:hover {
  color: #0071e3;
}

.login-btn {
  background: #0071e3;
  color: white;
  border: none;
  padding: 8px 16px;
  border-radius: 12px;
  font-size: 14px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s ease;
}

.login-btn:hover {
  background: #0056b3;
  transform: translateY(-1px);
}

.login-btn:active {
  transform: scale(0.98);
}

/* Mobile Menu */
.hamburger {
  display: none;
  flex-direction: column;
  cursor: pointer;
  position: relative;
}

.hamburger input {
  display: none;
}

.hamburger span {
  display: block;
  width: 24px;
  height: 2px;
  background: #1d1d1f;
  margin: 3px 0;
  transition: 0.3s;
}

.hamburger input:checked ~ span:nth-child(2) {
  opacity: 0;
}

.hamburger input:checked ~ span:nth-child(3) {
  transform: rotate(-45deg) translate(5px, -5px);
}

.hamburger input:checked ~ span:nth-child(4) {
  transform: rotate(45deg) translate(5px, 5px);
}

.mobile-menu {
  position: absolute;
  top: 40px;
  right: 0;
  background: rgba(255, 255, 255, 0.95);
  backdrop-filter: blur(12px);
  border-radius: 12px;
  padding: 20px;
  min-width: 200px;
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.1);
  opacity: 0;
  visibility: hidden;
  transform: translateY(-10px);
  transition: all 0.3s ease;
}

.hamburger input:checked ~ .mobile-menu {
  opacity: 1;
  visibility: visible;
  transform: translateY(0);
}

.mobile-menu a {
  display: block;
  text-decoration: none;
  color: #1d1d1f;
  padding: 8px 0;
  font-size: 16px;
}

.mobile-login-btn {
  width: 100%;
  background: #0071e3;
  color: white;
  border: none;
  padding: 12px;
  border-radius: 8px;
  margin-top: 12px;
  cursor: pointer;
}

/* Hero Section */
.hero {
  position: relative;
  height: 100vh;
  display: flex;
  align-items: center;
  justify-content: center;
  text-align: center;
  overflow: hidden;
}

.hero-background {
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background-image: url('https://images.unsplash.com/photo-1506905925346-21bda4d32df4?w=1920&q=80');
  background-size: cover;
  background-position: center;
  background-attachment: fixed;
  filter: brightness(0.7);
}

.hero-content {
  position: relative;
  z-index: 2;
  max-width: 800px;
  padding: 0 20px;
  color: white;
}

.hero-title {
  font-size: 56px;
  font-weight: 600;
  line-height: 1.1;
  margin-bottom: 24px;
  letter-spacing: -0.5px;
}

.hero-subtitle {
  font-size: 24px;
  font-weight: 400;
  margin-bottom: 40px;
  opacity: 0.9;
  line-height: 1.4;
}

.hero-actions {
  display: flex;
  gap: 16px;
  justify-content: center;
}

.cta-primary, .cta-secondary {
  padding: 12px 24px;
  border-radius: 12px;
  font-size: 16px;
  font-weight: 500;
  cursor: pointer;
  transition: all 0.2s ease;
  border: none;
}

.cta-primary {
  background: #0071e3;
  color: white;
}

.cta-primary:hover {
  background: #0056b3;
  transform: translateY(-1px);
}

.cta-secondary {
  background: rgba(255, 255, 255, 0.2);
  color: white;
  border: 1px solid rgba(255, 255, 255, 0.3);
}

.cta-secondary:hover {
  background: rgba(255, 255, 255, 0.3);
}

.cta-primary:active, .cta-secondary:active {
  transform: scale(0.98);
}

/* Info Section */
.info-section {
  padding: 120px 0;
  background: #f5f7fa;
}

.info-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
  gap: 40px;
}

.info-card {
  background: white;
  padding: 40px 32px;
  border-radius: 16px;
  text-align: center;
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.04);
  transition: all 0.3s ease;
}

.info-card:hover {
  transform: translateY(-4px);
  box-shadow: 0 12px 32px rgba(0, 0, 0, 0.08);
}

.info-icon {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  width: 64px;
  height: 64px;
  background: #0071e3;
  border-radius: 16px;
  color: white;
  margin-bottom: 24px;
}

.info-card h3 {
  font-size: 20px;
  font-weight: 600;
  margin-bottom: 12px;
  color: #1d1d1f;
}

.info-card p {
  font-size: 16px;
  color: #86868b;
  line-height: 1.5;
}

/* Features Section */
.features-section {
  padding: 120px 0;
}

.feature-row {
  display: flex;
  align-items: center;
  gap: 80px;
  margin-bottom: 120px;
}

.feature-row.reverse {
  flex-direction: row-reverse;
}

.feature-image {
  flex: 0 0 55%;
}

.feature-image img {
  width: 100%;
  height: 400px;
  object-fit: cover;
  border-radius: 16px;
  box-shadow: 0 12px 32px rgba(0, 0, 0, 0.1);
}

.feature-content {
  flex: 0 0 45%;
}

.feature-content h2 {
  font-size: 36px;
  font-weight: 600;
  margin-bottom: 24px;
  color: #1d1d1f;
  line-height: 1.2;
}

.feature-content p {
  font-size: 18px;
  color: #86868b;
  margin-bottom: 24px;
  line-height: 1.6;
}

.feature-content ul {
  list-style: none;
}

.feature-content li {
  font-size: 16px;
  color: #1d1d1f;
  margin-bottom: 8px;
  position: relative;
  padding-left: 24px;
}

.feature-content li::before {
  content: '✓';
  position: absolute;
  left: 0;
  color: #0071e3;
  font-weight: 600;
}

/* Footer */
.footer {
  background: #1d1d1f;
  color: white;
  padding: 80px 0 40px;
}

.footer-grid {
  display: grid;
  grid-template-columns: repeat(4, 1fr);
  gap: 40px;
  margin-bottom: 40px;
}

.footer-column h4 {
  font-size: 18px;
  font-weight: 600;
  margin-bottom: 20px;
  color: white;
}

.footer-column ul {
  list-style: none;
}

.footer-column li {
  margin-bottom: 8px;
}

.footer-column a {
  color: #86868b;
  text-decoration: none;
  font-size: 14px;
  transition: color 0.2s ease;
}

.footer-column a:hover {
  color: #0071e3;
}

.footer-bottom {
  border-top: 1px solid #424245;
  padding-top: 20px;
  text-align: center;
}

.footer-bottom p {
  color: #86868b;
  font-size: 14px;
}

/* Responsive Design */
@media (max-width: 768px) {
  .nav-menu {
    display: none;
  }

  .hamburger {
    display: flex;
  }

  .hero-title {
    font-size: 36px;
  }

  .hero-subtitle {
    font-size: 18px;
  }

  .hero-actions {
    flex-direction: column;
    align-items: center;
  }

  .cta-primary, .cta-secondary {
    width: 200px;
  }

  .info-grid {
    grid-template-columns: 1fr;
    gap: 24px;
  }

  .feature-row, .feature-row.reverse {
    flex-direction: column;
    gap: 40px;
  }

  .feature-image, .feature-content {
    flex: none;
  }

  .feature-content h2 {
    font-size: 28px;
  }

  .footer-grid {
    grid-template-columns: repeat(2, 1fr);
    gap: 24px;
  }
}

@media (max-width: 480px) {
  .hero-title {
    font-size: 28px;
  }

  .hero-subtitle {
    font-size: 16px;
  }

  .info-card {
    padding: 24px 20px;
  }

  .feature-content h2 {
    font-size: 24px;
  }

  .footer-grid {
    grid-template-columns: 1fr;
  }
}
</style>