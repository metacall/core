from django.http import HttpResponse

import os

def index(request):
	return HttpResponse('hello world');

def pid(request):
	return HttpResponse(str(os.getpid()));
