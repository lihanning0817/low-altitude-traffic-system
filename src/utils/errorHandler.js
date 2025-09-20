/**
 * 统一错误处理工具
 * 提供对 ResizeObserver 错误和其他常见错误的处理
 */

// 保存原始的 console.error 方法
const originalConsoleError = console.error;

// ResizeObserver 错误白名单
const resizeObserverIgnoredErrors = [
  'ResizeObserver loop',
  'ResizeObserver loop completed with undelivered notifications'
];

// 是否启用错误过滤
let errorFilteringEnabled = true;

/**
 * 初始化错误处理
 */
export function initErrorHandler() {
  // 重写 console.error 方法以过滤特定错误
  console.error = function (...args) {
    // 如果错误过滤被禁用，直接使用原始方法
    if (!errorFilteringEnabled) {
      return originalConsoleError.apply(console, args);
    }

    // 检查是否是需要忽略的错误
    if (args[0] && typeof args[0] === 'string') {
      const errorMessage = args[0];
      const shouldIgnore = resizeObserverIgnoredErrors.some(ignoredError =>
        errorMessage.includes(ignoredError)
      );

      if (shouldIgnore) {
        // 忽略这些错误，不输出到控制台
        return;
      }
    }

    // 对于其他错误，使用原始的 console.error 方法
    originalConsoleError.apply(console, args);
  };

  // 全局错误处理
  window.addEventListener('error', (event) => {
    // 忽略 ResizeObserver 相关的错误
    if (event.message && resizeObserverIgnoredErrors.some(error => event.message.includes(error))) {
      event.preventDefault();
      return false;
    }
  });

  // Promise 错误处理
  window.addEventListener('unhandledrejection', (event) => {
    // 忽略 ResizeObserver 相关的 Promise 错误
    if (event.reason && typeof event.reason === 'string') {
      const shouldIgnore = resizeObserverIgnoredErrors.some(error =>
        event.reason.includes(error)
      );

      if (shouldIgnore) {
        event.preventDefault();
        return false;
      }
    }
  });
}

/**
 * 启用错误过滤
 */
export function enableErrorFiltering() {
  errorFilteringEnabled = true;
}

/**
 * 禁用错误过滤
 */
export function disableErrorFiltering() {
  errorFilteringEnabled = false;
}

/**
 * 恢复原始的 console.error 方法
 */
export function restoreConsoleError() {
  console.error = originalConsoleError;
}

/**
 * 记录警告信息
 * @param {string} message - 警告信息
 * @param {any} data - 附加数据
 */
export function logWarning(message, data = null) {
  console.warn(`[系统警告] ${message}`, data);
}

/**
 * 记录错误信息
 * @param {string} message - 错误信息
 * @param {any} error - 错误对象
 * @param {any} data - 附加数据
 */
export function logError(message, error = null, data = null) {
  console.error(`[系统错误] ${message}`, error, data);
}

/**
 * 记录信息
 * @param {string} message - 信息
 * @param {any} data - 附加数据
 */
export function logInfo(message, data = null) {
  console.info(`[系统信息] ${message}`, data);
}

export default {
  initErrorHandler,
  enableErrorFiltering,
  disableErrorFiltering,
  restoreConsoleError,
  logWarning,
  logError,
  logInfo
};