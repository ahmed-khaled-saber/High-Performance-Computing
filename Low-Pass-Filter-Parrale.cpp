#include <mpi.h>
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include<string.h>
#include <ctime>
#include<msclr\marshal_cppstd.h>
#pragma once

#using <System.dll>
#using <mscorlib.dll>
#using <System.Drawing.dll>
#using <System.Windows.Forms.dll>

using namespace std;
using namespace msclr::interop;


 // Message Tags
const int Tag_Start_idx = 1;
const int Tag_Processor_Rows = 2;
const int Tag_Last_Processor = 3;
const int Tag_Filter_Size = 4;
const int Tag_Sub_Filtered = 5;
const int Tag_Sub_Size = 6;


"""
'Describtion...
تسكين الصورة ل كائن بيت ماب و تحويل الألوان أحمر أخضر أزرق إلي درجات رمادية
Params:
    @width: 
    @hieght:
    @img_path:
Returns:
    ...
"""
int* inputImage(int* w, int* h, System::String^ imagePath) 
{
    int* img_input;
    // احفظ الصورة كـ Bit Map
	System::Drawing::Bitmap bitmap(imagePath);

	*w = bitmap.Width;
	*h = bitmap.Height;
    // 
	int *Red   = new int[bitmap.Height * bitmap.Width];
	int *Green = new int[bitmap.Height * bitmap.Width];
	int *Blue  = new int[bitmap.Height * bitmap.Width];
	img_input  = new int[bitmap.Height * bitmap.Width];

    // تحويل كل بيكسل من الصورة إلي قيمة رمادية بمتوسط قيم قنوات الألوان الثلاثة
	for (int i = 0; i < bitmap.Height; i++) // row
	{
		for (int j = 0; j < bitmap.Width; j++) //col
		{
			System::Drawing::Color color = bitmap.GetPixel(j, i);
			Red[i * bitmap.Width + j] = color.R;
			Blue[i * bitmap.Width + j] = color.B;
			Green[i * bitmap.Width + j] = color.G;
			img_input[i*bitmap.Width + j] = ((color.R + color.B + color.G) / 3); //gray scale value equals the average of RGB values

		}
	}
	return img_input;

}


"""
'Describtion...
خلق صورة أبعادها كالأبعاد المُرسلة
Params:
    @img:
    @width: 
    @hieght:
    @index:
Returns:
    ...
"""
void createImage(int* img, int width, int height, int index)
{
    System::Drawing::Bitmap new_img(width, height);

	for (int i = 0; i < new_img.Height; i++) //row
	{
		for (int j = 0; j < new_img.Width; j++) //col
		{
			if (image[i*width + j] < 0) // no -ve's
			{
				image[i*width + j] = 0;
			}
			if (image[i*width + j] > 255) // cut-off
			{
				image[i*width + j] = 255;
			}
			System::Drawing::Color color = System::Drawing::Color::FromArgb(image[i*new_img.Width + j], image[i*new_img.Width + j], image[i*new_img.Width + j]); //grayscale
			new_img.SetPixel(j, i, color);
		}
	}
	new_img.Save("..//Data//Output//outputRes3" + index + ".png");
}

"""
'Describtion...
توسعة الصورة الأصلية بهوامش مُصاغة من حجم الفلتر
Params:
    @img:
    @width: 
    @height:
    @filter_size:
    @pad_size:
    @padded_width
Returns:
    ...
"""
int* pad_image(int* image, int width, int height, int filter_size, int pad_size, int padded_width)
{
    int*img_padded = new int[pad_size]();
    // مقدار الــ padding
	int half_size_filter = (filter_size - 1) / 2;
    // fill old pixels one by one to the new padded Image
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			img_padded[(i + half_size_filter) * (padded_width)+(j + half_size_filter)] = image[i * width + j];
		}
	}
	return img_padded;

}

"""
'Describtion...
Params:
    @img:
    @sub_size: الحجم من الصورة الأصلية الذي سيٌطبق عليه الفلتر في المرة الواحدة لمناداة هذة الفانكشن
    @height:
    @height:
    @start_idx:
    @Last_processor:
    @filter_size:
Returns:
    ...
"""
int* low_pass_filter(int* image, int sub_size, int height, int start_idx, bool Last_processor, int filter_size)
{
    // تهيأة للمٌخرج النهائي
    int* result = new int[sub_size]();

	int mask_size = pow(filter_size, 2);
	int* mask = new int(mask_size);

	int width = sub_size / height;

	int half_filter_size = (filter_size - 1) / 2;

	int start_height = 0;
    int end_height = height;

	int start_width = half_filter_size;
    int end_width = width - half_filter_size;

	if (start_idx == 0)
		start_height = half_filter_size;

	else if (Last_processor)
		end_height = height - half_filter_size;

	//Move mask through all elements of the image
	for (int h = start_height; h < end_height; h++) // rows
	{
		for (int w = start_width; w < end_width; w++) // cols
		{
			int start_mask_height = h - half_filter_size;
            int end_mask_height   = h + half_filter_size;

			int start_mask_width  = w - half_filter_size;
            int end_mask_width    = w + half_filter_size;

			// select mask elements
			int k = 0;
			for (int j = start_mask_height; j <= end_mask_height; j++)
			for (int i = start_mask_width; i <= end_mask_width; i++)
				mask[k++] = image[(j * width + i) + start_idx];

			int sum = 0;
			for (int i = 0; i < mask_size; i++)
				sum += mask[i];

			// Calc the average
			result[(h * width) + w] = sum / mask_size;
		}
	}
	return result;

}

"""
'Describtion...
Params:
    @args:
    @argv: 
Returns:
    ...
"""
int main(int args, char** argv)
{
    // Default IMG dims
    int ImageWidth = 4;
    int ImageHeight = 4;

    // Time Variables
	int start_s, stop_s, TotalTime = 0;

    // Master Node
    int master_id = 0;
	int ProcessorId, Processors_num;

    // Padded Image
	int padded_width;
    int padded_height;
    int padded_image_size;

    // Problem Decomposition 
	int rows_per_processor, start_idx;
	int sub_image_size;

	int filter_size;

	bool Last_processor = FALSE;

	int* imageData = NULL;
	int* padded_image = NULL;
	int* sub_filtered_image = NULL;
	int* result_image = NULL;


    MPI_Status RecvStatus;

    // prepare Image Path 
	System::String^ imagePath;
	std::string img;
	img = "..//Data//Input//test3.jpg";

	// record Time
    start_s = clock();

    // شقّي الرُحي INIT FINALIZE
    MPI_Init(NULL, NULL);
    // 
    
    // كل وحدة معالجة تلتقط مُعرِّفها
    int ProcessorId;
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcessorId);

    // لكى يتم تجزئة الصورة بالتساوي
    int size;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
    // 
    
    if (ProcessorId == master_id)
    { // WELCOME MASTER NODE, Distribute The Data(The Image) please!
		cout << "Enter number of processors (Master not working): ";
		cin >> Processors_num;

		cout << "Enter size of Filter (odd number): ";
		cin >> filter_size;

		// reading image
		cout << "... Reading image ...\n";
		imagePath = marshal_as<System::String^>(img);
		imageData = inputImage(&ImageWidth, &ImageHeight, imagePath);

		// padding image
		cout << "... Padding image...\n";
		padded_width = ImageWidth + filter_size - 1;
		padded_height = ImageHeight + filter_size - 1;
		padded_image_size = padded_width * padded_height;

		padded_image = new int[padded_image_size]();
		padded_image = pad_image(imageData, ImageWidth, ImageHeight, filter_size, padded_image_size, padded_width);

		//تجزأة الصورة بالعدل على عدد الوِحدات المركزية المساهمة ماعدا الأولي المركزية و الأخيرة تأخذ ما تبقي 
		rows_per_processor = padded_height / Processors_num - 1;

        // POINT TO POINT COMMUNICATION 

		//destribute The Data over all Processors
		for (int i = 1; i < Processors_num; i++)
		{
			// لكل بروسيسور مؤشر ليبدأ من أين ؟
			start_idx = (i - 1) * (padded_width * rows_per_processor);

            // إذا كان الأخير
			if (i == Processors_num - 1)
			{
				// last processor consume the rest, and raise a flag
				rows_per_processor = padded_height - (rows_per_processor * (Processors_num - 2));
				Last_processor = TRUE;
			}

			// للإحاطة بالإختلافات الطفيفة ..رحساب حجم الصورة المسؤول عن فلترتها كل وحدة معالجة
			sub_image_size = padded_width * rows_per_processor;

			// send data Via Disrtibuted Memory (Message Passing Model)
            '''
            لكل وحدة معالجة سيصلها
            عدد الصفوف المسؤولة عنها من الصورة و حجمها و مؤشر البدء في الصورة
            و إذا ما كان اخر وحدة معالجة صاحبة الحمل المغاير عن الباقي أم لا و حجم الفلتر
            '''
			MPI_Send(&rows_per_processor, 1, MPI_INT, i, Tag_Processor_Rows, MPI_COMM_WORLD);
			MPI_Send(&sub_image_size, 1, MPI_INT, i, Tag_Sub_Size, MPI_COMM_WORLD);
			MPI_Send(&start_idx, 1, MPI_INT, i, Tag_Start_idx, MPI_COMM_WORLD);
			MPI_Send(&Last_processor, 1, MPI_INT, i, Tag_Last_Processor, MPI_COMM_WORLD);
			MPI_Send(&filter_size, 1, MPI_INT, i, Tag_Filter_Size, MPI_COMM_WORLD);
		}
	}	// master work DONE!
    else //  كل غير السيِّد 
    {
        MPI_Recv(&rows_per_processor, 1, MPI_INT, master_id, Tag_Processor_Rows, MPI_COMM_WORLD, &RecvStatus);
		MPI_Recv(&sub_image_size, 1, MPI_INT, master_id, Tag_Sub_Size, MPI_COMM_WORLD, &RecvStatus);
		MPI_Recv(&start_idx, 1, MPI_INT, master_id, Tag_Start_idx, MPI_COMM_WORLD, &RecvStatus);
		MPI_Recv(&Last_processor, 1, MPI_INT, master_id, Tag_Last_Processor, MPI_COMM_WORLD, &RecvStatus);
		MPI_Recv(&filter_size, 1, MPI_INT, master_id, Tag_Filter_Size, MPI_COMM_WORLD, &RecvStatus);

    }

    // إعلام و إذاعة من اليد للجميع بحجم الصورة بعد التحديث ليقوموا بتهيئة مصفوفة مصممة خصيصا لهذا الحجم 
	MPI_Bcast(&padded_image_size, 1, MPI_INT, master_id, MPI_COMM_WORLD);

	if (ProcessorId != master_id)
		padded_image = new int[padded_image_size](); 
    // بعد التهيأة, يبث السييّد الصورة من لدته 
	MPI_Bcast(padded_image, padded_image_size, MPI_INT, master_id, MPI_COMM_WORLD);

    // TIME TO HARD/REAL WORK
    if (ProcessorId != master_id)
	{
        // أمر لكل الوِحدات ما عدا السيِّد بأن يُطبقوا الفلتر على جزأهم الخاص
		sub_filtered_image = new int[sub_image_size]();
		sub_filtered_image = low_pass_filter(padded_image, sub_image_size, rows_per_processor, start_idx, Last_processor, filter_size);

		// إعادة إرسال البيانات للسيد مع الصورة بعد التأثير عليها بالفلتر
		MPI_Send(&sub_image_size, 1, MPI_INT, master_id, Tag_Sub_Size, MPI_COMM_WORLD);
		MPI_Send(sub_filtered_image, sub_image_size, MPI_INT, master_id, Tag_Sub_Filtered, MPI_COMM_WORLD);
		MPI_Send(&start_idx, 1, MPI_INT, master_id, Tag_Start_idx, MPI_COMM_WORLD);
		cout << "Processor " << ProcessorId << " says..send filtered Img!" << endl;
	}
    else // السيِّد فقط 
	{
        // تهيئة صورة بالحجم الكامل
		result_image = new int[padded_image_size]();

        // ليلقف كل الإرسالات من الخوادم بعد تطبيق الفلتر يحتاج إلي لوب
		int finished_processor = 0;
		while (finished_processor < Processors_num - 1)
		{
			MPI_Recv(&sub_image_size, 1, MPI_INT, MPI_ANY_SOURCE, Tag_Sub_Size, MPI_COMM_WORLD, &RecvStatus);
            int Source = RecvStatus.MPI_SOURCE;

			sub_filtered_image = new int[sub_image_size]();
			
            MPI_Recv(sub_filtered_image, sub_image_size, MPI_INT, Source, Tag_Sub_Filtered, MPI_COMM_WORLD, &RecvStatus);
			MPI_Recv(&start_idx, 1, MPI_INT, Source, Tag_Start_idx, MPI_COMM_WORLD, &RecvStatus);

			// combine Sub-Solutions by Master Only
			int s = 0;
			for (int i = start_idx; i < start_idx + sub_image_size; ++i)
			{
				result_image[i] = sub_filtered_image[s++];

			}
			finished_processor++;
		}
        // IMAGE RENDER
		createImage(result_image, padded_width, padded_height, 0);

        // 
		stop_s = clock();
		TotalTime += (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
		cout << "time: " << TotalTime << "in Seconds. " << endl;
	}

    // Free Parallel Environment Variables
    free(imageData);
	free(padded_image);
	free(sub_filtered_image);
	free(result_image);

    MPI_Finalize();
    return 0;
}