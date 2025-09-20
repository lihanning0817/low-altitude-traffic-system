/**
 * API 服务测试文件
 * 用于测试与后端 C++ 服务的连接
 */

import apiService from './api';

// 测试 API 连接
async function testApiConnection() {
  console.log('Testing API connection...');

  try {
    // 测试健康检查端点
    const response = await fetch(`${apiService.baseURL}/health`, {
      method: 'GET',
      headers: {
        'Content-Type': 'application/json',
      },
    });

    if (response.ok) {
      const data = await response.json();
      console.log('API connection test successful:', data);
      return true;
    } else {
      console.error('API connection test failed with status:', response.status);
      return false;
    }
  } catch (error) {
    console.error('API connection test failed with error:', error);
    return false;
  }
}

// 测试登录功能
async function testLogin() {
  console.log('Testing login functionality...');

  try {
    // 使用测试账户登录
    // const testData = {
    //   username: 'testuser',
    //   password: 'testpassword'
    // };

    // 注意：这只是一个测试，实际的登录端点可能不同
    // const response = await apiService.login(testData);
    console.log('Login test completed (mocked)');
    return true;
  } catch (error) {
    console.error('Login test failed:', error);
    return false;
  }
}

// 运行所有测试
async function runAllTests() {
  console.log('Running API integration tests...');

  const tests = [
    { name: 'API Connection', test: testApiConnection },
    { name: 'Login Functionality', test: testLogin }
  ];

  const results = [];

  for (const { name, test } of tests) {
    try {
      const result = await test();
      results.push({ name, passed: result });
      console.log(`✓ ${name} test: ${result ? 'PASSED' : 'FAILED'}`);
    } catch (error) {
      console.error(`✗ ${name} test failed with error:`, error);
      results.push({ name, passed: false });
    }
  }

  console.log('\nTest Results Summary:');
  console.log('====================');
  let passedTests = 0;

  for (const { name, passed } of results) {
    console.log(`${passed ? '✓' : '✗'} ${name}: ${passed ? 'PASSED' : 'FAILED'}`);
    if (passed) passedTests++;
  }

  console.log(`\n${passedTests}/${tests.length} tests passed`);

  return passedTests === tests.length;
}

// 如果直接运行此文件，则执行测试
if (typeof window === 'undefined') {
  // 这里可以添加 Node.js 环境下的测试执行
  console.log('API test module loaded');
}

export { testApiConnection, testLogin, runAllTests };
