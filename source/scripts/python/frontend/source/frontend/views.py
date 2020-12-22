from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt

import os

def index(request):
	return HttpResponse('hello world')

def pid(request):
	return HttpResponse(str(os.getpid()))

@csrf_exempt
def media(request):
	if request.method == 'POST':
		text = request.FILES['file']

		return HttpResponse(text)
	else:
		return HttpResponseForbidden('Allowed only via POST')
