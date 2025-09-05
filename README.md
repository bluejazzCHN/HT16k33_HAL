# Usage sample
``` c
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();
	/* USER CODE BEGIN 2 */
	HT16K33_HandleTypeDef ht16k33;
	HT16K33_Init(&ht16k33, &hi2c1, 8);
	HT16K33_SetBrightness(&ht16k33, 5);
	HT16K33_Clear(&ht16k33);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
	       // 显示整数
	        HT16K33_DisplayInteger(&ht16k33, 1234);
	        HAL_Delay(2000);

	        // 显示浮点数，保留2位小数
	        HT16K33_DisplayFloat(&ht16k33, 12.34f, 2);
	        HAL_Delay(2000);

	        // 显示负数
	        HT16K33_DisplayFloat(&ht16k33, -5.67f, 2);
	        HAL_Delay(2000);

	        // 显示温度值
	        HT16K33_DisplayNumber(&ht16k33, 95.5, 1, SEGMENT_DEGREE); // 显示95.5°
	        HAL_Delay(2000);

	        // 设置闪烁效果
	        HT16K33_SetBlink(&ht16k33, 2); // 1Hz闪烁
	        HAL_Delay(5000);
	        HT16K33_SetBlink(&ht16k33, 0); // 停止闪烁
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

```
