from ast import Try
from cProfile import label
from curses import meta
from itertools import count

import matplotlib.pyplot as plt
import mpld3

import numpy as np
from pathlib import *

import os

class Plotter:

    def __init__(self,output_dir) -> None:
        self.output_dir=output_dir
        if not os.path.isdir(output_dir):
            os.mkdir(output_dir)

    #
    # METHODS TO PLOT COUNTERS' MEASUREMENTS
    #

    def plotCountervsEntropy(self,df):
        pass

    #
    # METHODS TO PLOT CLASSIFICATION METRICS
    #

    def PlotAllDatasets(self,metrics_list,paths,model_type,architectural):

        num=len(paths)

        fig, axs = plt.subplots(1, num, sharey=True, tight_layout=True)

        fig.suptitle('Metrics of all the datasets for model: ' + model_type)

        for i in range(num):

            axs[i].set_title(paths[i],fontsize=4)
            curr_metrics=metrics_list[i]
            axs[i].bar(['A'],
                [curr_metrics['accuracy']],
                color='red'
            )
            axs[i].bar(['P'],
                [curr_metrics['precision']],
                color='blue'
            )
            axs[i].bar(['R'],
                [curr_metrics['recall']],
                color='green'
            )
            axs[i].grid()
        
        if architectural:
            output_path=Path(self.output_dir).joinpath('Sumary_arch_'+ model_type+'.png')
        else:
            output_path=Path(self.output_dir).joinpath('Sumary_'+ model_type+'.png')

        plt.grid()
        #plt.savefig(output_path,format='png')
        plt.show()
    
    def PlotMetricAcrossModels(self,matrices,models,metric):

        n=len(models)
        levels=['VERY_LOW','LOW','MED.','HIGH']
        plots=[]

        fig, axs = plt.subplots(1, n, sharey=True, tight_layout=False)

        for i,m in enumerate(models):
            axs[i].set_title(f"{metric} of {m}")
            axs[i].set_xlabel('Entropy')
            axs[i].set_ylabel('Activity')
            axs[i].set_xticks(np.arange(len(levels)), labels=levels, fontsize=5)
            axs[i].set_yticks(np.arange(len(levels)), labels=levels)
            p=axs[i].imshow(matrices[i],cmap='Greys',vmin=0,vmax=1)
            plots.append(p)

            for j in range(len(levels)):
                for k in range(len(levels)):
                    text = axs[i].text(k, j, round(matrices[i][j][k],3) ,ha="center", va="center", color="w", fontsize=10)

        
        fig.set_size_inches(10,10)
        plt.colorbar(plots[0],cax=None,ax=axs,orientation='horizontal')
        #plt.savefig(output_path,format='png')
        plt.show()

    def PlotAllDatasetsMatrix(self,matrix_a, matrix_p, matrix_r, matrix_f, model_type,options):

        levels=['VERY_LOW','LOW','MED.','HIGH']
        
        num=4
        fig, axs = plt.subplots(1, 4, sharey=True, tight_layout=False)

        option_str=''

        for op in options:

            if options.index(op)>0:
                if op=='Architectural':
                    op += ' Counters'
                option_str += ', '+op 
            else:
                option_str+=op
        

        fig.suptitle('Metrics of all the datasets for model: ' + model_type )

        if len(options)>0:
            fig.supxlabel('Using '+option_str)

        axs[0].set_title('Accuracy')
        axs[1].set_title('Precision')
        axs[2].set_title('Recall')
        axs[3].set_title('F1')

        for i in range(num):
            axs[i].set_xlabel('Entropy')
            axs[i].set_ylabel('Activity')
            axs[i].set_xticks(np.arange(len(levels)), labels=levels)
            axs[i].set_yticks(np.arange(len(levels)), labels=levels)

        ca=axs[0].imshow(matrix_a,cmap='Greys',vmin=0,vmax=1)
        cp=axs[1].imshow(matrix_p,cmap='Greys',vmin=0,vmax=1)
        cr=axs[2].imshow(matrix_r,cmap='Greys',vmin=0,vmax=1)
        cf=axs[3].imshow(matrix_f,cmap='Greys',vmin=0,vmax=1)

        for i in range(len(levels)):
            for j in range(len(levels)):
                text = axs[1].text(j, i, round(matrix_p[i][j],3) ,ha="center", va="center", color="w", fontsize=10)
                text = axs[2].text(j, i, round(matrix_r[i][j],3) ,ha="center", va="center", color="w", fontsize=10)
                text = axs[3].text(j, i, round(matrix_f[i][j],3) ,ha="center", va="center", color="w", fontsize=10)
        
        fig.set_size_inches(10,10)
        #plt.colorbar(ca,ax=axs[0],orientation='horizontal')
        #plt.colorbar(cp,ax=axs[1],orientation='horizontal')
        plt.colorbar(cr,cax=None,ax=axs,orientation='horizontal')
        #plt.savefig(output_path,format='png')
        plt.show()
        pass

    def PlotImportantCounters(self,recalls,models):
        
        logistic_rec=[]
        dt_rec=[]
        svm_rec=[]


        for i in range(0,len(recalls),3):
            logistic_rec.append(recalls[i])
            dt_rec.append(recalls[i+1])
            svm_rec.append(recalls[i+2])

        fig, axs = plt.subplots(1, 1, sharey=True, tight_layout=True)

        fig.suptitle('Keeping the n most important counters')

        axs.set_xlabel("n")
        axs.set_ylabel("Recall")
        axs.plot( list(range(1,1+len(logistic_rec))),logistic_rec,label='Logistic' )
        axs.plot( list(range(1,1+len(logistic_rec))),dt_rec,label='DecisionTree' )
        axs.plot( list(range(1,1+len(logistic_rec))),svm_rec,label='SVM' )
        axs.grid()
        axs.legend()

        plt.show()


    def PlotMetrics(self,accuracies,precisions,recalls,models,n):
        """
        Plots the target metrics
        """

        fig, axs = plt.subplots(1, 3, sharey=True, tight_layout=True)

        fig.suptitle('Metrics after keeping the '+ str(n) +' most relevant counters')

        axs[0].set_title('Accuracy')
        axs[0].bar( models,accuracies,color='red' )
        axs[0].grid()

        axs[1].set_title('Precision')
        axs[1].bar( models,precisions, color='blue' )
        axs[1].grid()

        axs[2].set_title('Recall')
        axs[2].bar( models,recalls, color='green' )
        axs[2].grid()

        plt.show()


    def PlotAccPrecRec(self,accuracies,precisions,recalls,model_name,metaparam):
        """
        Given a list precisions of tuples (model_type,precision),
        plots them in a histogram.
        """
        variances=[p[0] for p in accuracies]
        accuracy_list=[p[1] for p in accuracies]
        precision_list=[p[1] for p in precisions]
        recall_list=[p[1] for p in recalls]

        fig, axs = plt.subplots(1, 3, sharey=True, tight_layout=True)

        fig.suptitle('PCA effect on metrics: '+ model_name)
        fig.text(0,0,s=metaparam)

        axs[0].set_title('Accuracy')
        axs[0].set_xlabel('Variance kept')
        axs[0].plot( variances,accuracy_list,'r' )
        axs[0].grid()

        axs[1].set_title('Precision')
        axs[1].set_xlabel('Variance kept')
        axs[1].plot( variances,precision_list, 'b' )
        axs[1].grid()

        axs[2].set_title('Recall')
        axs[2].set_xlabel('Variance kept')
        axs[2].plot( variances,recall_list, 'g' )
        axs[2].grid()


        output_path=Path(self.output_dir).joinpath('PCA_'+ model_name+'.png')
        plt.savefig(output_path,format='png')
        plt.show()

    def counterSubset2Int(self,subset,whole):
        """
        Converts a subset of counters as an int number
        """
        res=0

        for i in range(len(whole)):
            curr_counter=whole[i]

            if curr_counter in subset:
                res |= (1<< len(whole)-i-1)
        return res
    
    def int2CounterSubset(self,n,whole):
        """
        Convert an integer to a subset of counters
        """
        res=""

        for i in range(len(whole)):

            bit = (n>>i) & 1

            if bit==1:
                res+=whole[i]+"\n"
        
        return res


    def PlotCountersDrop(self,accuracies,precisions,recalls,counters_dropped,num_dropped,model_name,metaparam):
        """
        Plots the variations in the target metrics as a 
        function of the dropped counters-
        """

        fig, axs = plt.subplots(1, 1, sharey=True, tight_layout=True)

        fig.suptitle('Effects of dropping columns on: '+ model_name)
        fig.text(0,0,s=metaparam)

        axs.set_title('Dropping ' + str(num_dropped)+ 'label(s) at a time')
        axs.set_xlabel('counters dropped')
        axs.plot( counters_dropped, accuracies,label='Accuracy')
        axs.plot( counters_dropped, precisions,label='Precision' )
        axs.plot( counters_dropped, recalls, label='Recall' )
        axs.grid()

        output_path=Path(self.output_dir).joinpath('Drop_'+str(num_dropped)+'_counters_'+ model_name+'.png')
        plt.legend()

        html_str = mpld3.fig_to_html(fig)
        Html_file= open(Path(self.output_dir).joinpath('Drop_'+str(num_dropped)+'_counters_'+ model_name+'.html'),"w")
        Html_file.write(html_str)
        Html_file.close()

        plt.xticks(counters_dropped,counters_dropped)
        plt.savefig(output_path,format='png')
        plt.show()

    def GetCountersBestMetric(self,metric_list,metric_best,metric_worst,counters_dropped,n):
        """
        Samples the n best and n worst counters dropped for the metric represented in
        metric_list
        """
        best=[]
        worst=[]
        best_i=0
        worst_i=0

        for i in range(len(metric_list)):
            if best_i< n and metric_list[i]==metric_best[best_i]:
                best.append(counters_dropped[i])
                best_i+=1
                continue

            if worst_i < n and metric_list[i]==metric_worst[worst_i]:
                worst.append(counters_dropped[i])
                worst_i+=1
                continue
        
        assert len(best) == n and len(worst) == n, f"Length error"
        return best,worst

    def plotCountersDropSample(self,accuracies,precisions,recalls,counters_dropped,num_dropped,percentage,model_name,metaparam):
        """
        Plots the variations in the target metrics as a 
        function of the dropped counters. Unlike plotCountersDrop,
        it only samples some of the best and the worst results.
        """

        n = len(accuracies)
        to_sample = round(n*percentage)

        unsort_acc=accuracies
        unsort_prec=precisions
        unsort_rec=recalls

        accuracies.sort()
        precisions.sort()
        recalls.sort()

        counters_acc=self.GetCountersBestMetric(unsort_acc, accuracies[-to_sample:],accuracies[0:to_sample],counters_dropped,to_sample)
        counters_prec=self.GetCountersBestMetric(unsort_prec, precisions[-to_sample:],precisions[0:to_sample],counters_dropped,to_sample)
        counters_rec=self.GetCountersBestMetric(unsort_rec, recalls[-to_sample:],recalls[0:to_sample],counters_dropped,to_sample)

        sampled_acc=accuracies[0:to_sample] + accuracies[-to_sample:]
        sampled_prec=precisions[0:to_sample] + precisions[-to_sample:]
        sampled_rec=recalls[0:to_sample] + recalls[-to_sample:]

        fig, axs = plt.subplots(1, 1, sharey=True, tight_layout=True)

        fig.suptitle('Effects of dropping columns on: '+ model_name)

        axs.set_title('Dropping ' + str(num_dropped)+ 'label(s) at a time,' +str(to_sample) + ' best and worst')
        axs.set_xlabel('counters dropped')

        axs.plot( counters_acc[1] +counters_acc[0], sampled_acc,'r',label='Accuracy')
        axs.plot( counters_prec[1]+counters_prec[0],sampled_prec,'b',label='Precision' )
        axs.plot( counters_rec[1]+counters_rec[0], sampled_rec,'g', label='Recall' )
        axs.grid()

        output_path=Path(self.output_dir).joinpath('Drop_'+str(num_dropped)+'_counters-sapmples_'+ model_name+'.png')
        plt.legend()

        html_str = mpld3.fig_to_html(fig)
        Html_file= open(Path(self.output_dir).joinpath('Drop_'+str(num_dropped)+'_counters-samples_'+ model_name+'.html'),"w")
        Html_file.write(html_str)
        Html_file.close()

        plt.savefig(output_path,format='png')
        plt.show()

        labels=['DWT_LSUCNT', 'DWT_CPICNT', 'DWT_EXCCNT', 'DWT_CYCCNT', 'DWT_SLEEPCNT','DWT_FOLDCNT', 'LSU_stim', 'CPI_stim', 'FLD_stim', 'EXC_stim', 'TIME']
        self.barCountersDropSample(labels,sampled_acc,sampled_prec,sampled_rec,counters_acc,counters_prec, counters_rec, num_dropped,model_name,metaparam)
        print(counters_acc)
        print(counters_prec)
        print(counters_rec)
    
    def barCountersDropSample(self,labels,accuracies,precisions,recalls,sampled_acc,sampled_prec,sampled_rec,num_dropped,model_name,metaparam):
        
        lab_size=5
        fig, axs = plt.subplots(1, 1, sharey=True, tight_layout=True)
        yticks_list=[x/100 for x in range(0,105,5)]

        fig.suptitle('Effects of dropping columns on: '+ model_name)
        fig.text(0,0,s=metaparam)
        fig.set_size_inches(18.5,10.5)

        axs.grid(axis='y')
        axs.set_title('Dropping ' + str(num_dropped)+ 'label(s) at a time,' + str(len(accuracies)//2) + ' best and worst')
        axs.set_xlabel('counters dropped')

        axs.bar( [self.int2CounterSubset(x,labels) for x in sampled_rec[0] + sampled_rec[1]],recalls,color='green')
        axs.set_ylabel('Recall')
        axs.set_yticks(yticks_list)

        output_path=Path(self.output_dir).joinpath('BAR_RECALL_Drop_'+str(num_dropped)+'_counters-samples_'+ model_name+'.png')

        plt.savefig(output_path,format='png')
        plt.xticks(fontsize=lab_size)
        plt.show()

        fig, axs = plt.subplots(1, 1, sharey=True, tight_layout=True)

        fig.suptitle('Effects of dropping columns on: '+ model_name)
        fig.text(0,0,s=metaparam)
        fig.set_size_inches(18.5,10.5)

        axs.grid(axis='y')
        axs.set_title('Dropping ' + str(num_dropped)+ ' label(s) at a time,' + str(len(accuracies)//2) + ' best and worst')
        axs.set_xlabel('counters dropped')

        axs.bar( [self.int2CounterSubset(x,labels) for x in sampled_prec[0] + sampled_prec[1]],precisions,color='blue')
        axs.set_ylabel('Precision')
        axs.set_yticks(yticks_list)

        output_path=Path(self.output_dir).joinpath('BAR_PRECISION_Drop_'+str(num_dropped)+'_counters-samples_'+ model_name+'.png')

        plt.savefig(output_path,format='png')
        plt.xticks(fontsize=lab_size)
        plt.show()

        fig, axs = plt.subplots(1, 1, sharey=True, tight_layout=True)

        fig.suptitle('Effects of dropping columns on: '+ model_name)
        fig.text(0,0,s=metaparam)
        fig.set_size_inches(18.5,10.5)

        axs.grid(axis='y')
        axs.set_title('Dropping ' + str(num_dropped)+ 'label(s) at a time,' + str(len(accuracies)//2) + ' best and worst')
        axs.set_xlabel('counters dropped')

        axs.bar( [self.int2CounterSubset(x,labels) for x in sampled_acc[0] + sampled_acc[1]],accuracies,color='red')
        axs.set_ylabel('Accuracy')
        axs.set_yticks(yticks_list)

        output_path=Path(self.output_dir).joinpath('BAR_ACCURACY_Drop_'+str(num_dropped)+'_counters-samples_'+ model_name+'.png')

        plt.savefig(output_path,format='png')
        plt.xticks(fontsize=lab_size)
        plt.show()

    def PlotPrecRecallScatter(self,precisions,recalls):

        fig, axs = plt.subplots(1, 1, sharey=True, tight_layout=True)

        #fig.suptitle('Effects of dropping columns on: '+ model_name)
        logistic_prec=[]
        logistic_rec=[]
        decision_prec=[]
        decision_rec=[]
        svm_prec=[]
        svm_rec=[]

        for i in range(len(precisions)):

            if i %3==0:
                logistic_prec.append(precisions[i])
                logistic_rec.append(recalls[i])
            
            if i %3==1:
                decision_prec.append(precisions[i])
                decision_rec.append(recalls[i])

            if i %3==2:
                svm_prec.append(precisions[i])
                svm_rec.append(recalls[i])

        axs.set_title('Scatter Precision vs Recall')
        axs.set_xlabel('Recall')

        axs.plot( logistic_rec,logistic_prec,'bs',label='Log')
        axs.plot( svm_rec,svm_prec,'ro',label='SVM')
        axs.plot( decision_rec,decision_prec,'gv',label='DT')
        axs.grid()

        plt.legend()
        plt.show()


    def PlotInteraction(self,absent_metrics,present_metrics):

        num_counters=len(absent_metrics.keys())
        fig, axs = plt.subplots(1, 11, sharey=True, tight_layout=True)

        fig.suptitle('DROPPING COUNTERS')

        i=0
        j=0
        for counter in absent_metrics:

            #print(axs[j][i])
            #axs[j].set_title('Effect of dropping',counter)
            axs[i].set_title('Dropping '+counter, fontdict={'fontsize':5})
            axs[i].set_xlabel('Recall')
            axs[i].set_xticks([0.0,1.0])
            axs[i].set_yticks([0.0,0.2,0.4,0.6,0.8,1.0])

            avg_pres=np.average(present_metrics[counter])
            avg_not_pres=np.average(absent_metrics[counter])
            var_pres=np.std(present_metrics[counter])
            var_not_pres=np.std(absent_metrics[counter])

            axs[i].errorbar( [0,1],[avg_pres,avg_not_pres],yerr=[var_pres,var_not_pres],ecolor = 'green', elinewidth = 2, capsize=5,label='Log')
            axs[i].grid()

            # Update indexes
            i+=1
            

        plt.xticks([0.0,1.0],fontsize=4)
        plt.yticks([0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0])
        plt.legend()
        plt.show()

    def PlotImportances(self,averages,stds,counters,model_type):
        """
        Plots the importance of the input counters
        """

        fig, axs = plt.subplots(1, 1, sharey=True, tight_layout=True)

        fig.suptitle('Importance of counters for ' + model_type)

        avgs=[averages[c] for c in averages]
        stds=[stds[c] for c in stds]
        axs.errorbar(counters,avgs,yerr=stds,ecolor = 'green', elinewidth = 2, capsize=5)
        plt.xticks(fontsize=5)
        plt.grid()
        plt.show()

     
    #
    # METHODS TO PLOT CONST. MALWARE SIZE
    #

    def PlotConstMalwareSize(self,metric_lists,model_name,metaparam):

        fig, axs = plt.subplots(1, 1, sharey=True, tight_layout=True)

        fig.suptitle('Metrics when malware size stays constant')
        fig.text(0,0,s=metaparam)

        accuracies=[]
        precisions=[]
        recalls=[]
        sizes=[]
        i=1
        for m in metric_lists:
            accuracies.append(m['accuracy'])
            precisions.append(m['precision'])
            recalls.append(m['recall'])
            sizes.append(i)
            i+=1

        axs.set_title('Trying ' + str(len(metric_lists))+ ' malware size ranges on model:',model_name)
        axs.set_xlabel('Size range')
        axs.plot( sizes, accuracies,label='Accuracy')
        axs.plot( sizes, precisions,label='Precision' )
        axs.plot( sizes, recalls, label='Recall' )
        axs.grid()

        output_path=Path(self.output_dir).joinpath('malware_sizes_'+ model_name+'.png')
        plt.legend()

        html_str = mpld3.fig_to_html(fig)
        Html_file= open(Path(self.output_dir).joinpath('malware_sizes_'+ model_name+'.html'),"w")
        Html_file.write(html_str)
        Html_file.close()

        plt.xticks(sizes,sizes)
        plt.savefig(output_path,format='png')
        plt.show()
    
    def PlotFeatureSelectionTable(self,counters,data,model_type):
        
        content=[]
        metaparam_combinations=[]

        fig, ax = plt.subplots(
            1,
            1,
            figsize=(25,25)
        )

        for key in data.keys():

            combination='('+key[0]+','+key[1]+')'
            metaparam_combinations.append(combination)

            counter_list=data[key]
            new_row=[' ']*len(counters)

            for c in counter_list:
                i=counters.index(c)
                new_row[i]='X'
            
            content.append(new_row)

        # hide axes
        fig.patch.set_visible(False)
        ax.axis('off')
        #ax.axis('tight')
        #plt.rcParams['figure.figsize'] = [30,30]

        tab=ax.table(
            cellText=content,
            rowLabels=metaparam_combinations,
            colLabels=counters,
            cellLoc='center',
        )

        tab.set_fontsize(54)
        tab.scale(5,10)

        for key, cell in tab.get_celld().items():
            cell.set_linewidth(8)
            cell.set_fontsize(42)

        plt.tight_layout()
        #plt.title(f"Counter importance for model: {model_type}")

        plt.savefig( Path(self.output_dir).joinpath(f'{model_type}_IterativeImportance.png') , format='png') 
        plt.show()





