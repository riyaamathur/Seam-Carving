#include <stdio.h>
#include "c_img.h"
#include "seamcarving.h"
#include <math.h>


void PrintCostArrNicely(double* cost_array, int w, int h)
{
    for (int y = 0; y < h; y++)
    {
        for (int x = 0; x < w; x++)
        {
            printf("%f ", cost_array[y * w + x]);
        }
        printf("\n");
    }
}

void PrintPath(int* path, int len)
{
	for (int i = 0; i < len; i++)
	{
		printf("%d ", path[i]);
	}
	printf("\n");
}

int wrap(int x, int max)
{
    if (x < 0) return max - 1;
    else if (x >= max) return 0;
    else return x;
}

int clip(int x, int max)
{
	if (x < 0) return 0;
	else if (x >= max) return max - 1;
	else return x;
}


void calc_energy(struct rgb_img* im, struct rgb_img** grad)
{
    create_img(grad, im->height, im->width);

    int w = im->width;
    int h = im->height;

    for (int x = 0; x < im->width; x++)
    {
        for (int y = 0; y < im->height; y++)
        {
            int r_x = get_pixel(im, y, wrap(x + 1, w), 0) - get_pixel(im, y, wrap(x - 1, w), 0);
            int g_x = get_pixel(im, y, wrap(x + 1, w), 1) - get_pixel(im, y, wrap(x - 1, w), 1);
            int b_x = get_pixel(im, y, wrap(x + 1, w), 2) - get_pixel(im, y, wrap(x - 1, w), 2);

            int r_y = get_pixel(im, wrap(y + 1, h), x, 0) - get_pixel(im, wrap(y - 1, h), x, 0);
            int g_y = get_pixel(im, wrap(y + 1, h), x, 1) - get_pixel(im, wrap(y - 1, h), x, 1);
            int b_y = get_pixel(im, wrap(y + 1, h), x, 2) - get_pixel(im, wrap(y - 1, h), x, 2);

            
            int delta_x = r_x * r_x + g_x * g_x + b_x * b_x;
            int delta_y = r_y * r_y + g_y * g_y + b_y * b_y; 
            
            int energy = (uint8_t) (sqrt(delta_x + delta_y) / 10);

            set_pixel(*grad, y, x, energy, energy, energy);



        }
    }
}


void dynamic_seam(struct rgb_img* grad, double** best_arr)
{
	*best_arr = (double*)malloc(sizeof(double) * grad->height * grad->width);

	for (int x = 0; x < grad->width; x++)
	{
		(*best_arr)[x] = get_pixel(grad, 0, x, 0);
	}

	int w = grad->width;
	int h = grad->height;
    
	for (int y = 1; y < grad->height; y++)
	{
		for (int x = 0; x < grad->width; x++)
		{
			double min = (*best_arr)[(y - 1) * w + clip(x - 1, w)];
            
			if ((*best_arr)[clip((y - 1), h) * w + clip(x,     w)] < min) min = (*best_arr)[clip((y - 1), h) * w + clip(x,     w)];
		
			if ((*best_arr)[clip((y - 1), h) * w + clip(x + 1, w)] < min) min = (*best_arr)[clip((y - 1), h) * w + clip(x + 1, w)];

			(*best_arr)[y * w + x] = min + (double)get_pixel(grad, y, x, 0);
		}
	} 
}

void recover_path(double *best, int height, int width, int **path)
{
	*path = (int*)malloc(sizeof(int) * height);
    
	int min = best[width * (height - 1)];
	int min_x = 0;
	
	for (int x = 1; x < width; x++)
	{
		if (best[width * (height - 1) + x] < min)
		{
			min = best[width * (height - 1) + x];
			min_x = x;
		}
	}
	
	(*path)[height - 1] = min_x;

	for (int y = height - 1; y > 0; y--)
	{
		int curmin_x = min_x;
		min = best[width * (y - 1) + clip(min_x, width)];

		if (best[width * (y - 1) + clip(curmin_x - 1, width)] < min)
		{
			min = best[width * (y - 1) + clip(curmin_x - 1, width)];
			min_x = clip(curmin_x - 1, width);
		}

		if (best[width * (y - 1) + clip(curmin_x + 1, width)] < min)
		{
			min = best[width * (y - 1) + clip(curmin_x + 1, width)];
			min_x = clip(curmin_x + 1, width);
			
		}
		(*path)[y - 1] = min_x;
	}
}


void remove_seam(struct rgb_img* src, struct rgb_img** dest, int* path)
{
	create_img(dest, src->height, src->width - 1);


	
	for (int y = 0; y < src->height; y++)
	{
		for (int x = 0; x < src->width; x++)
		{
			if (x < path[y])
			{
				set_pixel(*dest, y, x, get_pixel(src, y, x, 0), get_pixel(src, y, x, 1), get_pixel(src, y, x, 2));
			}
			else if (x > path[y])
			{
				set_pixel(*dest, y, x - 1, get_pixel(src, y, x, 0), get_pixel(src, y, x, 1), get_pixel(src, y, x, 2));
			}
		}
	}
	
	
}

/*
int main()
{

	struct rgb_img* im;
	struct rgb_img* cur_im;
	struct rgb_img* grad;
	double* best;
	int* path;

	read_in_img(&im, "HJoceanSmall.bin");

	for (int i = 0; i < 5; i++) {
		printf("i = %d\n", i);
		calc_energy(im, &grad);
		dynamic_seam(grad, &best);
		recover_path(best, grad->height, grad->width, &path);
		remove_seam(im, &cur_im, path);

		char filename[200];
		sprintf(filename, "img%d.bin", i);
		write_img(cur_im, filename);


		destroy_image(im);
		destroy_image(grad);
		free(best);
		free(path);
		im = cur_im;
	}
	destroy_image(im);

}

*/