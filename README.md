# 基于STM32 HAL库的ST7789 LCD屏幕驱动

## 简介

本项目基于中景园电子的驱动代码改进，简单实现了以下功能

        1.硬件初始化
        2.绘制像素点
        3.绘制任意角度直线
        4.绘制字符及字符串
        5.绘制有符号数

## 改进

本项目尝试优化了中景园电子的代码可读性和可维护性

        1.硬件初始化选项可高度自定义
        2.利用字符表实现了跳过刷新重复字符
        3.函数实现清晰可读

## 使用说明

使用 `git clone` 即可将本项目克隆到本地，将源文件添加到项目中即可使用

## 补充

如有任何改进的提议，均可提出`Issue`或是`Pull request`
