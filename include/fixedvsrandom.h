unsigned int tracenumber, instructions, leakyinstructionno = 0;

FILE * loadtrace(int trace){
    
    FILE *fp;
    char str[500], filepath[500];
    
    strcpy(str, TRACEFOLDER);
    strcat(str, TRACEFILE);
    sprintf(filepath, str, trace);
    fp = fopen(filepath, "r");
    
    if(fp == NULL){
        
        printf("Error\n");
        
    }
    
    return fp;
    
}

//-------------------------------------------------------------------

int gettracelength(FILE *fp){
    
    int lines = 0;
    double ch;
    
    while(!feof(fp))
    {
        
        ch = fread(&ch, sizeof(double), 1, fp);
        lines++;
    }
    
    return lines;
}

//-------------------------------------------------------------------

void meancenter(double *trace, int len){
    
    int i;
    double average;
    
    average = 0;
    
    for(i=0;i<len;i++)
        average += trace[i];
    
    average = average/len;
    
    for(i=0;i<len;i++)
        trace[i] -= average;
    
}

//-------------------------------------------------------------------

void gettrace(double *ptr, FILE *fp, int len, int *points){
    int i, j;
    double value, *traceptr;
    
    traceptr = malloc(tracelength*sizeof(double));
    
    for(i=0;i<tracelength;i++)
    {
#ifdef BINARYTRACES
        fread(&value,sizeof(double),1,fp);
#else
        fscanf(fp,"%lf",&value);
#endif
        traceptr[i] = value;
    }
    
    meancenter(traceptr,tracelength);
    
    j = 0;
    
    for(i=0;i<tracelength;i++){
        if(points[j] == i){
            ptr[j] = traceptr[i];
            //printf("%lf\n", ptr[j]);
            j++;
        }
    }
    free(traceptr);
}

//-------------------------------------------------------------------

void multiplypoints(double *trace, double *multipliedtrace, int len){
    
    int i,j,t=0;
    
    for(i=0;i<len;i++)
        for(j=0;j<len;j++,t++){
            multipliedtrace[t] = trace[i]*trace[j];
            //printf("POINT %d: %f %f\n", t, multipliedtrace[t], trace[j]);
        }
}

//-------------------------------------------------------------------

void getaverage(double *average, int len, int start, int end, int *points, int order){
    
    int i, j, len_final;
    FILE *fp;
    double *temp, *finaltrace;
    temp = malloc(len*sizeof(double));
    
    if(order == 1){
        len_final = len;
        finaltrace = temp;
    }
    else{
        len_final = ((len*len));
        finaltrace = malloc(len_final*sizeof(double));
    }

    for(i=start;i<end;i++){
        
        fp = loadtrace(i);
        gettrace(temp, fp, len, points);
        fclose(fp);

        if(order>1){
            multiplypoints(temp, finaltrace, len);
        }
        
        for(j=0;j<len_final;j++){
            average[j] += finaltrace[j];
        }
    }

    for(j=0;j<len_final;j++){
        average[j] /= tracenumber;
       // printf("%0.20f\n", average[j]);
    }
    free(finaltrace);
}

//-------------------------------------------------------------------

void getvariance(double *variance, int len, double *average, int start, int end, int *points, int order){
    
    int i, j, len_final;
    FILE *fp;
    double *temp, *finaltrace;
    temp = malloc(len*sizeof(double));
    
    if(order == 1){
        len_final = len;
        finaltrace = temp;
    }
    else{
        len_final = ((len*len));
        finaltrace = malloc(len_final*sizeof(double));
    }
    
    for(i=start;i<end;i++){
        
        fp = loadtrace(i);
        gettrace(temp, fp, len, points);
        fclose(fp);
        
        if(order>1){
            multiplypoints(temp, finaltrace, len);
        }
        
        for(j=0;j<len_final;j++){
            variance[j] += pow((finaltrace[j] - average[j]),2);
        }
    }
    
    for(j=0;j<len_final;j++){
        variance[j] /= (tracenumber-1);
    }
    free(finaltrace);
}

//-------------------------------------------------------------------

void getttest(double *ttest, double *randomaverage, double *randomvariance, double *fixedaverage, double *fixedvariance, int len){
    
    int i;
    // FILE *fp;
    
    //fp = fopen(FIXEDVSRANDOMTTEST, "w");

    for(i=0;i<len;i++){

        ttest[i] = ((fixedaverage[i] - randomaverage[i]) / (sqrt((fixedvariance[i]/tracenumber)+(randomvariance[i]/tracenumber))));
        //printf("%d: Fixed: %lf %lf Random: %lf %lf\n\n",i, fixedaverage[i], fixedvariance[i], randomaverage[i], randomvariance[i]);

        //printf("%0.20f\n", randomvariance[i]);
    }

  //  fclose(fp);

}

//-------------------------------------------------------------------

void getmaskpoints(int *maskpoint, int mask, int len_masktrace){
    
    FILE *fp;
    fp = fopen(KEYFLOWOUTPUTFILE, "r");
    int op1_mask, op2_mask, i, j, prevmaskpoint;
    
    j = 0, prevmaskpoint=0;
    
    for(i=0;i<tracelength;i++)
    {
        fscanf(fp,"%d %d",&op1_mask, &op2_mask);
        if(op1_mask == mask || op2_mask == mask){
            if(prevmaskpoint == i){
                maskpoint[j] = i+1;
                j+=1;
            }
            else{
                maskpoint[j] = i;
                maskpoint[j+1] = i+1;
                j+=2;
            }
            prevmaskpoint = i+1;
        }
    }
    fclose(fp);

}

//-------------------------------------------------------------------

int getmaskpointslen(int mask){
    
    FILE *fp;
    fp = fopen(KEYFLOWOUTPUTFILE, "r");
    int op1_mask, op2_mask, i, j, prevmaskpoint;
    
    j = 0, prevmaskpoint=0;
    
    for(i=0;i<tracelength;i++)
    {
        fscanf(fp,"%d %d",&op1_mask, &op2_mask);
        if(op1_mask == mask || op2_mask == mask){
            if(prevmaskpoint == i){
                j+=1;
            }
            else{
                j+=2;
            }
        prevmaskpoint = i+1;
        }
    }
    fclose(fp);
    return(j);
    
}

//-------------------------------------------------------------------

unsigned int getmasklen(int mask){
    
    FILE *fp;
    fp = fopen(KEYFLOWOUTPUTFILE, "r");
    int op1_mask, op2_mask, i, j;
    
    j = 0;
    
    for(i=0;i<tracelength;i++)
    {
        fscanf(fp,"%d %d",&op1_mask, &op2_mask);
        if(op1_mask == mask || op2_mask == mask){
            j++;
        }
    }
    
    fclose(fp);
    
    return j;
    
}

//-------------------------------------------------------------------
void firstorderfixedvsrandom(void){
    FILE *fp;
    double *fixedaverage, *randomaverage, *fixedvariance, *randomvariance, *ttest;
    int i, *points;
    
    // Create variables
    fixedaverage = malloc(tracelength*sizeof(double));
    randomaverage = malloc(tracelength*sizeof(double));
    fixedvariance = malloc(tracelength*sizeof(double));
    randomvariance = malloc(tracelength*sizeof(double));
    ttest = malloc(tracelength*sizeof(double));
    points = malloc(tracelength*sizeof(int));
    
    for (i=0;i<tracelength;i++){
        fixedaverage[i] = 0;
        randomaverage[i] = 0;
        fixedvariance[i] = 0;
        randomvariance[i] = 0;
        ttest[i] = 0;
        points[i] = i;
    }

    // Get random average and variance. Fixed average is same as trace and variance is 0.
    getaverage(fixedaverage, tracelength, 1, tracenumber+1, points, 1);
    getaverage(randomaverage, tracelength, tracenumber+1, (tracenumber*2)+1, points, 1);
    getvariance(fixedvariance, tracelength, fixedaverage, 1, tracenumber+1, points, 1);
    getvariance(randomvariance, tracelength, randomaverage, tracenumber+1, (tracenumber*2)+1, points, 1);
    getttest(ttest, randomaverage, randomvariance, fixedaverage, fixedvariance, tracelength);
    
    fp = fopen(FIXEDVSRANDOMFILE, "w+");
    
    // Index from 1
    
    for(i=0;i<tracelength;i++){
        fprintf(fp, "%f\n", ttest[i]);
        if(ttest[i] < -FIXEDVSRANDOMFAIL | ttest[i] > FIXEDVSRANDOMFAIL){
            leakyinstructionno++;
        }
    }
    fclose(fp);
    free(fixedaverage);
    free(randomaverage);
    free(fixedvariance);
    free(randomvariance);
    free(ttest);
    free(points);

}

//-------------------------------------------------------------------

void secondorderfixedvsrandom(int len_masktrace, int mask){
    
    //printf("%d\n", len_masktrace);

    
    FILE *fp;
    double *fixedaverage, *randomaverage, *fixedvariance, *randomvariance, *ttest;
    int i, *maskpoint,leakyinstructionnomask=0, len_multipliedmasktrace;
    char *temp;
    
    temp = malloc(100);

    len_multipliedmasktrace = len_masktrace*len_masktrace;
    printf("%d\n\n", len_multipliedmasktrace);

    // Create variables
    fixedaverage = malloc(len_multipliedmasktrace*sizeof(double));
    randomaverage = malloc(len_multipliedmasktrace*sizeof(double));
    fixedvariance = malloc(len_multipliedmasktrace*sizeof(double));
    randomvariance = malloc(len_multipliedmasktrace*sizeof(double));
    ttest = malloc(len_multipliedmasktrace*sizeof(double));
    maskpoint = malloc(len_multipliedmasktrace*sizeof(int));
    
    for (i=0;i<len_multipliedmasktrace;i++){
        fixedaverage[i] = 0;
        randomaverage[i] = 0;
        fixedvariance[i] = 0;
        randomvariance[i] = 0;
        ttest[i] = 0;
        maskpoint[i] = 0;
    }

    // +1 as 0 is no mask
    getmaskpoints(maskpoint, mask+1, len_masktrace);

    // Get random average and variance. Fixed average is same as trace and variance is 0.
    getaverage(fixedaverage, len_masktrace, 1, tracenumber+1, maskpoint, 2);
    getaverage(randomaverage, len_masktrace, tracenumber+1, (tracenumber*2)+1, maskpoint, 2);
    getvariance(fixedvariance, len_masktrace, fixedaverage, 1, tracenumber+1, maskpoint, 2);
    getvariance(randomvariance, len_masktrace, randomaverage, tracenumber+1, (tracenumber*2)+1, maskpoint, 2);
    getttest(ttest, randomaverage, randomvariance, fixedaverage, fixedvariance, len_multipliedmasktrace);
    
    sprintf(temp, "%smask%d.txt", FIXEDVSRANDOMFILE, mask);
    fp = fopen(temp, "w+");
    
    //Index from 1
    
    for(i=0;i<len_multipliedmasktrace;i++){
        fprintf(fp, "%f\n", ttest[i]);
        if(ttest[i] < -FIXEDVSRANDOMFAIL | ttest[i] > FIXEDVSRANDOMFAIL){
            printf("%d %d\n", i/len_masktrace, i%len_masktrace);
            leakyinstructionnomask++;
        }
    }

    fclose(fp);
    
    maskfixedvsrandomfail[mask] = leakyinstructionnomask;
    free(fixedaverage);
    free(randomaverage);
    free(fixedvariance);
    free(randomvariance);
    free(ttest);
    free(maskpoint);
    return;
}

//-------------------------------------------------------------------

void fixedvsrandom(void){
    
    FILE *fp;
    int i, len;

    printf("\n########################################\n\nSTARTING FIRST ORDER FIXED VS RANDOM T-TEST...\n\n");
    
    // Get trace length
    fp = loadtrace(1);
    tracelength = gettracelength(fp);
    instructions = tracelength;
    fclose(fp);
    
    firstorderfixedvsrandom();

    printf("%d LEAKY INSTRUCTIONS/CYCLES FOUND\n", leakyinstructionno);
    
    printf("\n########################################\n\nSTARTING SECOND ORDER FIXED VS RANDOM T-TEST...\n\n");
    
    no_masks = 0;
#ifdef KEYFLOW
    while(getmasklen(no_masks+1)!=0)
        no_masks++;
#endif
    if(no_masks == 0){
        printf("NO MASKS TO ANALYSE\n\n");
        return;
    }
    
    maskfixedvsrandomfail = malloc(no_masks*sizeof(int));

    for(i=0;i<no_masks;i++){
        printf("ANALYSING MASK NUMBER %d\n", i);
        secondorderfixedvsrandom(getmaskpointslen(i+1), i);
    }
    free(maskfixedvsrandomfail);

}

//-------------------------------------------------------------------
